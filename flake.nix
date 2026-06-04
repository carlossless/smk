{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-26.05";
    utils.url = "github:numtide/flake-utils";
    sinowealth-kb-tool = {
      url = "github:carlossless/sinowealth-kb-tool";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs = { self, nixpkgs, utils, sinowealth-kb-tool }:
    utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs {
          inherit system;
        };

        # uCsim (the SDCC 8051 simulator) patched so the SH68F90's USB interrupt
        # (vector 7 @ 0x3B) is a real, registered interrupt source — stock uCsim
        # only knows the classic 8051 vectors. Lets us drive usb_interrupt_handler
        # in simulation with faithful vectoring/stack/nesting.
        ucsim-sh68f90 = pkgs.stdenv.mkDerivation {
          pname = "ucsim-sh68f90";
          inherit (pkgs.sdcc) version;

          src = pkgs.sdcc.src;

          # bison/flex are required; the SDCC tarball does not ship the generated
          # parser/lexer.
          nativeBuildInputs = with pkgs; [ bison flex ];

          # SH68F90 is a proper uCsim CPU variant (-t sh68f90). This small patch
          # only REGISTERS it (type enum + cpus_51[] row + sim51.cc factory case +
          # objs.mk) -- no edits to shared s51 core logic. The actual CPU and
          # peripheral model is the standalone sh68f90.cc / sh68f90cl.h copied in
          # below.
          patches = [ ./tools/ucsim/sh68f90-register.patch ];

          # Drop in the SH68F90 variant sources, and generate cmdlex.cc by hand
          # (uCsim's Makefile declares it as a target with no recipe, so flex never
          # runs and an empty lexer slips through -> link fails on yylex/uc_yy_*).
          postPatch = ''
            cp ${./tools/ucsim/sh68f90.cc}  sim/ucsim/src/sims/s51.src/sh68f90.cc
            cp ${./tools/ucsim/sh68f90cl.h} sim/ucsim/src/sims/s51.src/sh68f90cl.h
            flex -o sim/ucsim/src/core/cmd.src/cmdlex.cc \
                    sim/ucsim/src/core/cmd.src/cmdlex.l
          '';

          # Build only the mcs51 simulator (others, e.g. f8, fail to build and we
          # don't need them).
          configurePhase = ''
            runHook preConfigure
            cd sim/ucsim
            ./configure \
              --enable-avr-sim=no   --enable-f8-sim=no     --enable-i8048-sim=no \
              --enable-i8085-sim=no --enable-m6800-sim=no  --enable-m6809-sim=no \
              --enable-m68hc08-sim=no --enable-m68hc11-sim=no --enable-m68hc12-sim=no \
              --enable-mos6502-sim=no --enable-oisc-sim=no --enable-p1516-sim=no \
              --enable-pblaze-sim=no --enable-pdk-sim=no   --enable-rxk-sim=no \
              --enable-st7-sim=no   --enable-stm8-sim=no   --enable-tlcs-sim=no \
              --enable-xa-sim=no    --enable-z80-sim=no
            runHook postConfigure
          '';

          buildPhase = ''
            runHook preBuild
            make
            runHook postBuild
          '';

          # Install under a distinct name: nixpkgs' sdcc already ships a stock
          # `ucsim_51` (no USB patch), so a same-named binary would collide on PATH.
          installPhase = ''
            runHook preInstall
            mkdir -p $out/bin
            cp src/sims/s51.src/ucsim_51 $out/bin/ucsim-sh68f90
            runHook postInstall
          '';

          meta.description = "SDCC uCsim 8051 simulator patched with the SH68F90 USB interrupt source";
        };

        # SDCC's source-level debugger. nixpkgs' sdcc build does NOT ship it, so we
        # build it from the same 4.5.0 source -- this matters because sdcdb talks to
        # uCsim over a socket and an older (4.2.0) sdcdb hangs against our 4.5.0
        # ucsim variant. Only ./configure (for config.h) + the mcs51 debugger are
        # built; the full compiler isn't needed.
        sdcdb = pkgs.stdenv.mkDerivation {
          pname = "sdcdb";
          inherit (pkgs.sdcc) version src;
          nativeBuildInputs = with pkgs; [ bison flex python3 pkg-config ];
          buildInputs = with pkgs; [ boost zlib ];
          # sdcdb's .cdb reader leaves the trailing newline on every record
          # (so module names become "<name>\n" and no source loads), and its
          # filename-search fallback spins forever on '_'-containing names.
          # Both bite our 4.5.0 .cdb -> sdcdb hangs at startup without this.
          patches = [ ./tools/ucsim/sdcdb-fix.patch ];
          configurePhase = ''
            runHook preConfigure
            ./configure --disable-pic14-port --disable-pic16-port --disable-device-lib
            runHook postConfigure
          '';
          buildPhase = ''
            runHook preBuild
            # sdcdb is 1999-era C; build with an older standard (its `typedef short
            # bool` clashes with C23's `bool` keyword) and -fcommon for its
            # tentative-definition globals.
            make -C debugger/mcs51 CC="gcc -std=gnu99 -fcommon"
            runHook postBuild
          '';
          installPhase = ''
            runHook preInstall
            mkdir -p $out/bin
            cp bin/sdcdb $out/bin/
            runHook postInstall
          '';
          meta.description = "SDCC source-level debugger (sdcdb), built from 4.5.0 source to match the uCsim variant";
        };
      in
      {
        packages = {
          inherit ucsim-sh68f90 sdcdb;
        };

        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            sdcc
            meson
            ninja

            sinowealth-kb-tool.packages."${system}".default # flashing
            clang-tools # for clang-format

            ucsim-sh68f90 # patched 8051 simulator (USB vector 7 + SIE model)
            python3       # USB simulator test suite (tests/)

            # useful supplementary tools
            binutils # for objcopy
            uhubctl
          ];
        };
      }
    );
}
