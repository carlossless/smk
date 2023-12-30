CC= sdcc
ASM = sdas8051
SDAR ?= sdar rc
OBJCOPY = objcopy
PACKIHX = packihx
FLASHER = sinowealth-kb-tool write -p nuphy-air60

SRCDIR = src
OBJDIR = obj
BINDIR = bin

FAMILY = mcs51
PROC = mcs51

FREQ_SYS ?= 24000000
XRAM_SIZE ?= 0x1000
XRAM_LOC ?= 0x0000
CODE_SIZE ?= 0xf000 # 61440 bytes (leaving the remaining 4096 for bootloader)

SMK_VERSION ?= alpha

# Ease backup & restore process by keeping same vid & pid as nuphy-air60
USB_VID ?= 0x05ac
USB_PID ?= 0x024f

DEBUG ?= 1

CFLAGS := -V -mmcs51 --model-small \
	--xram-size $(XRAM_SIZE) --xram-loc $(XRAM_LOC) \
	--code-size $(CODE_SIZE) \
	--std-c2x \
	-I$(ROOT_DIR)../include \
	-DDEBUG=$(DEBUG) \
	-DFREQ_SYS=$(FREQ_SYS) \
	-DWATCHDOG_ENABLE=1 \
	-DUSB_VID=$(USB_VID) \
	-DUSB_PID=$(USB_PID) \
	-DSMK_VERSION=$(SMK_VERSION)
LFLAGS := $(CFLAGS)
AFLAGS := -plosgff

# TODO: this should be selected based on the target being built
LAYOUT_SOURCES := $(wildcard $(SRCDIR)/keyboards/nuphy-air60/layouts/default/*.c)
# main.c has to be the first file
MAIN_SOURCES := $(SRCDIR)/main.c \
	$(filter-out $(SRCDIR)/main.c, $(wildcard $(SRCDIR)/*.c)) \
	$(LAYOUT_SOURCES)
MAIN_OBJECTS := $(MAIN_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.rel)

LIB_SOURCES := $(wildcard $(SRCDIR)/lib/*.c)
LIB_OBJECTS := $(LIB_SOURCES:$(SRCDIR)/lib/%.c=$(OBJDIR)/lib/%.rel)

.PHONY: all clean flash

all: $(BINDIR)/main.hex

clean:
	rm -rf $(BINDIR) $(OBJDIR)

flash: $(BINDIR)/main.hex
	$(FLASHER) $(BINDIR)/main.hex

$(OBJDIR)/%.rel: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) -m$(FAMILY) -l$(PROC) $(CFLAGS) -c $< -o $@

$(BINDIR)/main.lib: $(LIB_OBJECTS)
	@mkdir -p $(@D)
	$(SDAR) $@ $^

$(BINDIR)/main.ihx: $(MAIN_OBJECTS) $(BINDIR)/main.lib
	@mkdir -p $(@D)
	$(CC) -m$(FAMILY) -l$(PROC) $(LFLAGS) -o $@ $^

$(BINDIR)/%.hex: $(BINDIR)/%.ihx
	${PACKIHX} < $< > $@

$(BINDIR)/%.bin: $(BINDIR)/%.ihx
	$(OBJCOPY) -I ihex -O binary $< $@
