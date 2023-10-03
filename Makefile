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
XRAM_SIZE ?= 0x0400
XRAM_LOC ?= 0x0000
CODE_SIZE ?= 0xf000 # 61440 bytes (leaving the remaining 4096 for bootloader)

CFLAGS := -V -mmcs51 --model-small \
	--xram-size $(XRAM_SIZE) --xram-loc $(XRAM_LOC) \
	--code-size $(CODE_SIZE) \
	-I$(ROOT_DIR)../include -DFREQ_SYS=$(FREQ_SYS) -DWATCHDOG_ENABLE=1
LFLAGS := $(CFLAGS)

AFLAGS= -plosgff

SOURCES := $(SRCDIR)/main.c $(filter-out $(SRCDIR)/main.c, $(wildcard $(SRCDIR)/*.c)) # main.c has to be the first file
OBJECTS := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.rel)

.PHONY: all clean flash

all: $(BINDIR)/main.hex

clean:
	rm -rf $(BINDIR) $(OBJDIR)

flash: $(BINDIR)/main.hex
	$(FLASHER) $<

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
