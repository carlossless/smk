//! smk-console: print SMK's HID debug console output.
//!
//! SMK (DEBUG builds) ships debug text as HID input reports with report id
//! `REPORT_ID_CONSOLE` (7) on its "extra" interface. This tool opens every HID
//! device matching the given VID:PID and prints the payload of any report 7 it
//! sees, picking up and dropping matching devices as they appear or disappear.
//!
//! On connect it sends a `SET_REPORT(Feature, 7)` handshake: the firmware holds
//! its console output buffered until a tool announces itself this way, then
//! flushes everything queued so far (including the boot banner), defeating the
//! attach race where the host would drain a one-shot report first.
//!
//! ```sh
//! cargo run --release               # defaults to 05ac:024f (nuphy-air60)
//! cargo run --release -- 258a:002a  # eyooso-z11
//! ```
//!
//! On Linux the `/dev/hidraw*` node it reads is privileged: give it a udev
//! rule, or run the tool under `sudo`.

use std::collections::HashSet;
use std::fmt;
use std::io::{self, Write};
use std::process::ExitCode;
use std::str::FromStr;
use std::sync::{Arc, Mutex};
use std::thread;
use std::time::Duration;

use clap::Parser;
use hidra::{HidDevice, HidError, HidResult, Hidra, MaybeFuture};

const REPORT_ID_CONSOLE: u8 = 7;
const HANDSHAKE: [u8; 5] = [REPORT_ID_CONSOLE, b'S', b'M', b'K', 0];

const CONSOLE_USAGE_PAGE: u16 = 0xff31;
const CONSOLE_USAGE: u16 = 0x74;

const DEFAULT_DEVICE: &str = "05ac:024f";

const RESCAN: Duration = Duration::from_millis(500);

/// Print SMK's HID debug console output.
#[derive(Parser)]
#[command(version, about)]
struct Args {
    /// Device to watch, as a hex VID:PID
    #[arg(default_value = DEFAULT_DEVICE)]
    device: DeviceId,
}

#[derive(Clone, Copy)]
struct DeviceId {
    vid: u16,
    pid: u16,
}

impl FromStr for DeviceId {
    type Err = String;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        let (vid, pid) = s.split_once(':').ok_or("expected VID:PID")?;
        Ok(Self {
            vid: u16::from_str_radix(vid, 16).map_err(|err| format!("vendor id: {err}"))?,
            pid: u16::from_str_radix(pid, 16).map_err(|err| format!("product id: {err}"))?,
        })
    }
}

impl fmt::Display for DeviceId {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{:04x}:{:04x}", self.vid, self.pid)
    }
}

fn main() -> ExitCode {
    let args = Args::parse();

    match watch(args.device) {
        Ok(()) => ExitCode::SUCCESS,
        Err(err) => {
            eprintln!("[smk-console] {err}");
            ExitCode::FAILURE
        }
    }
}

fn watch(device: DeviceId) -> HidResult<()> {
    let api = Hidra::builder().enumerate_on_build(false).build()?;
    eprintln!("[smk-console] watching for {device} (ctrl-c to quit)");

    let attached = Arc::new(Mutex::new(HashSet::new()));
    let mut reported = HashSet::new();

    loop {
        for path in console_paths(&api, device)? {
            if !attached.lock().unwrap().insert(path.clone()) {
                continue;
            }
            match api.open_path(&path).wait() {
                Ok(handle) => {
                    reported.remove(&path);
                    let attached = Arc::clone(&attached);
                    thread::spawn(move || {
                        read_console(&path, &handle);
                        attached.lock().unwrap().remove(&path);
                    });
                }
                Err(err) => {
                    attached.lock().unwrap().remove(&path);
                    if reported.insert(path.clone()) {
                        eprintln!("[smk-console] {path}: {err}");
                    }
                }
            }
        }
        thread::sleep(RESCAN);
    }
}

fn console_paths(api: &Hidra, device: DeviceId) -> HidResult<Vec<String>> {
    let devices = api.enumerate(device.vid, device.pid)?;

    let mut paths: Vec<String> = devices
        .iter()
        .filter(|dev| dev.usage_page() == CONSOLE_USAGE_PAGE && dev.usage() == CONSOLE_USAGE)
        .map(|dev| dev.path().to_owned())
        .collect();

    // Windows reports an interface under its first top-level collection only,
    // so the console usage never shows up there; fall back to every match and
    // let the report id do the filtering, as the hidraw-only tool always did.
    if paths.is_empty() {
        paths = devices.iter().map(|dev| dev.path().to_owned()).collect();
    }

    paths.sort_unstable();
    paths.dedup();
    Ok(paths)
}

fn read_console(path: &str, device: &HidDevice) {
    eprintln!("[smk-console] connected {path}");

    // Interfaces picked up by the fallback above have no console collection and
    // reject the handshake; nothing to report, they just stay silent.
    let _ = device.send_feature_report(&HANDSHAKE).wait();

    let mut buf = [0u8; 64];
    loop {
        let len = match device.read(&mut buf).wait() {
            Ok(len) => len,
            Err(HidError::Disconnected) => break,
            Err(err) => {
                eprintln!("[smk-console] {path}: {err}");
                break;
            }
        };
        if len == 0 || buf[0] != REPORT_ID_CONSOLE {
            continue;
        }

        let payload = &buf[1..len];
        let end = payload
            .iter()
            .position(|&b| b == 0)
            .unwrap_or(payload.len());
        let text = &payload[..end];

        let mut out = io::stdout().lock();
        if out.write_all(text).and_then(|()| out.flush()).is_err() {
            break;
        }
    }

    eprintln!("[smk-console] disconnected {path}");
}
