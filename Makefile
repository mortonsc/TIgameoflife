CC=sdcc
CFLAGS=-mz80 --std-sdcc99 --reserve-regs-iy --max-allocs-per-node 30000
ASM=sdasz80
AFLAGS=-p -g -o
HEADERS=inc/ti83plus.h inc/fastcopy.h

game_of_life.8xp: game_of_life.bin
	binpac8x.py -O conway game_of_life.bin

game_of_life.bin: game_of_life.ihx
	hex2bin game_of_life.ihx

game_of_life.ihx: $(HEADERS) game_of_life.c inc/tios_crt0.rel inc/ti83plus.rel \
								inc/fastcopy.rel
	$(CC) $(CFLAGS) --code-loc 0x9D9B --data-loc 0 --no-std-crt0 \
			inc/tios_crt0.rel inc/ti83plus.rel inc/fastcopy.rel game_of_life.c

inc/ti83plus.rel: inc/ti83plus.asm
	$(ASM) $(AFLAGS) inc/ti83plus.asm

inc/fastcopy.rel: inc/fastcopy.asm
	$(ASM) $(AFLAGS) inc/fastcopy.asm

inc/tios_crt0.rel: inc/tios_crt0.s
	$(ASM) $(AFLAGS) inc/tios_crt0.s

.PHONY: clean
clean:
	rm -f game_of_life.8xp game_of_life.bin game_of_life.bin game_of_life.ihx
	rm -f *.asm *.lst *.sym *.lk *.map *.noi *.rel
	rm -f inc/*.rel inc/*.sym inc/*.lk inc/*.map inc/*.noi
