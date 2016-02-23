CC=sdcc
CFLAGS=-mz80 --std-sdcc99 --reserve-regs-iy --max-allocs-per-node 30000
LKFLAGS=--code-loc 0x9D9B --data-loc 0 --no-std-crt0
EXEC=game_of_life
INCS=inc/tios_crt0.rel inc/ti83plus.rel inc/fastcopy.rel

.PHONY: all inc clean

all: $(EXEC).8xp

$(EXEC).8xp: $(EXEC).bin
	binpac8x.py -O conway $(EXEC).bin

$(EXEC).bin: $(EXEC).ihx
	hex2bin $(EXEC).ihx

$(EXEC).ihx: $(EXEC).c inc
	$(CC) $(CFLAGS) $(LKFLAGS) $(INCS) $(EXEC).c

inc:
	$(MAKE) -C inc

clean:
	rm -f *.8xp *.bin *.ihx *.asm *.lst *.sym *.lk *.map *.noi *.rel
	$(MAKE) -C inc clean
