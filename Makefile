CC= sdcc
ASM = sdas8051
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

CFLAGS := -V -mmcs51 --model-small \
	--xram-size $(XRAM_SIZE) --xram-loc $(XRAM_LOC) \
	--code-size $(CODE_SIZE) \
	--std-c2x \
	-I$(ROOT_DIR)../include \
	-DDEBUG=1 \
	-DFREQ_SYS=$(FREQ_SYS) \
	-DWATCHDOG_ENABLE=1 \
	-DUSB_VID=$(USB_VID) \
	-DUSB_PID=$(USB_PID) \
	-DSMK_VERSION=$(SMK_VERSION)
LFLAGS := $(CFLAGS)

AFLAGS= -plosgff

SOURCES := $(SRCDIR)/main.c $(filter-out $(SRCDIR)/main.c, $(wildcard $(SRCDIR)/*.c)) # main.c has to be the first file
OBJECTS := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.rel)

.PHONY: all clean flash

all: $(BINDIR)/main.hex

clean:
	rm -rf $(BINDIR) $(OBJDIR)

flash: $(BINDIR)/main.hex
	$(FLASHER) $(BINDIR)/main.hex

$(OBJDIR)/%.rel: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) -m$(FAMILY) -l$(PROC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.rel: $(SRCDIR)/%.asm
	${ASM} ${AFLAGS} $@ $<

$(BINDIR)/main.ihx: $(OBJECTS) $(OBJDIR)/preboot.rel
	@mkdir -p $(@D)
	$(CC) -m$(FAMILY) -l$(PROC) $(LFLAGS) -o $@ $^

$(BINDIR)/%.hex: $(BINDIR)/%.ihx
	${PACKIHX} < $< > $@

$(BINDIR)/%.bin: $(BINDIR)/%.ihx
	$(OBJCOPY) -I ihex -O binary $< $@
