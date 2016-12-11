CC=sdcc
CFLAGS=-mz80 --std-sdcc99 --reserve-regs-iy --max-allocs-per-node 30000
LKFLAGS=--code-loc 0x9D9B --data-loc 0 --no-std-crt0
EXEC=conway
LDIR=lib

.PHONY: all lib clean release

all: $(EXEC).8xp

release: $(EXEC).zip

$(EXEC).zip: $(EXEC).8xp $(EXEC).c $(LDIR)/c_ti83p.h lib README.md LICENSE.txt
	zip $(EXEC).zip $(EXEC).8xp $(EXEC).c $(LDIR)/c_ti83p.h $(LDIR)/c_ti83p.lib \
		release_Makefile README.md LICENSE.txt
	# rename release_Makefile to Makefile in the zip archive
	printf "@ release_Makefile\n@=Makefile" | zipnote -w $(EXEC).zip

$(EXEC).8xp: $(EXEC).bin
	binpac8x.py -O conway $(EXEC).bin

$(EXEC).bin: $(EXEC).ihx
	sdobjcopy -Iihex -Obinary $(EXEC).ihx $(EXEC).bin

$(EXEC).ihx: $(EXEC).c lib
	$(CC) $(CFLAGS) $(LKFLAGS) $(LDIR)/tios_crt0.rel $(LDIR)/c_ti83p.lib $(EXEC).c

lib:
	$(MAKE) -C $(LDIR)

clean:
	rm -f *.8xp *.bin *.ihx *.asm *.lst *.sym *.lk *.map *.noi *.rel *.zip
	$(MAKE) -C $(LDIR) clean
