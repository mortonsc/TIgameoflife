CC=sdcc
CFLAGS=-mz80 --std-sdcc99 --reserve-regs-iy --max-allocs-per-node 30000
LKFLAGS=--code-loc 0x9D9B --data-loc 0 --no-std-crt0
EXEC=conway
LDIR=lib

# make all does not automatically recompile the library
# if the library is updated, it is necessary to make lib

.PHONY: all lib clean

all: $(EXEC).8xp

$(EXEC).8xp: $(EXEC).bin
	binpac8x.py -O conway $(EXEC).bin

$(EXEC).bin: $(EXEC).ihx
	sdobjcopy -Iihex -Obinary $(EXEC).ihx $(EXEC).bin

$(EXEC).ihx: $(EXEC).c lib
	$(CC) $(CFLAGS) $(LKFLAGS) $(LDIR)/tios_crt0.rel $(LDIR)/c_ti83p.lib $(EXEC).c

lib:
	make -C $(LDIR)

clean:
	rm -f *.8xp *.bin *.ihx *.asm *.lst *.sym *.lk *.map *.noi *.rel
	$(MAKE) -C $(LDIR) clean
