MANCHA ?= ..
MCC = $(MANCHA)/compilador_c/bin/mcc
MONTADOR = $(MANCHA)/simulador_completo/bin/montador
SIMULADOR = $(MANCHA)/simulador_completo/bin/simulador
BIOS = $(MANCHA)/bios_mancha/src/bios.asm

all: so.mob

so.asm: so.c so.h
	$(MCC) so.c -o so.asm

usuario.asm: usuario.c so.h
	$(MCC) usuario.c -o usuario.asm

so.mob: $(BIOS) ponte.asm ulib.asm so.asm usuario.asm
	$(MONTADOR) $(BIOS) ponte.asm ulib.asm so.asm usuario.asm -o so.mob

run: so.mob
	$(SIMULADOR) so.mob

clean:
	rm -f so.asm usuario.asm so.mob

.PHONY: all run clean
