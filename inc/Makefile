ASM=sdasz80
AFLAGS=-p -g -o

.PHONY: all clean

all: ti83plus.rel fastcopy.rel tios_crt0.rel

ti83plus.rel: ti83plus.asm ti83plus.inc
	$(ASM) $(AFLAGS) ti83plus.asm

fastcopy.rel: fastcopy.asm
	$(ASM) $(AFLAGS) fastcopy.asm

tios_crt0.rel: tios_crt0.s
	$(ASM) $(AFLAGS) tios_crt0.s

clean:
	rm -f *.rel *.lst *.sym *.lk *.map *.noi
