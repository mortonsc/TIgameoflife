CC=sdcc
CFLAGS=-mz80 --std-sdcc99 --reserve-regs-iy --max-allocs-per-node 30000
ASM=sdasz80
AFLAGS=-p -g -o

tigol.8xp: tigol.bin
	binpac8x.py -O tigol tigol.bin

tigol.bin: tigol.ihx
	hex2bin tigol.ihx

tigol.ihx: tigol.c inc/tios_crt0.rel inc/ti84plus.rel inc/fastcopy.rel
	$(CC) $(CFLAGS) --code-loc 0x9D9B --data-loc 0 --no-std-crt0 \
		inc/tios_crt0.rel inc/ti84plus.rel inc/fastcopy.rel tigol.c

inc/ti84plus.rel: inc/ti84plus.c
	$(CC) $(CFLAGS) -c inc/bkupmem.rel inc/ti84plus.c -o  inc/ti84plus.rel

inc/fastcopy.rel: inc/fastcopy.asm
	$(ASM) $(AFLAGS) inc/fastcopy.asm

inc/tios_crt0.rel: inc/tios_crt0.s
	$(ASM) $(AFLAGS) inc/tios_crt0.s

.PHONY: clean
clean:
	rm -f tigol.8xp tigol.bin tigol.bin tigol.ihx
	rm -f *.asm *.lst *.sym *.lk *.map *.noi *.rel
	rm -f inc/*.rel inc/*.sym inc/*.lk inc/*.map inc/*.noi
	rm -f inc/ti84plus.asm
