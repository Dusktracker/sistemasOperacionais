
João Guerra (202512648) e Gustavo Ribeiro (202512653)

 == comandos para a compilação: ==

make

compilador_c/bin/mcc -Icompilador_c/inc aleatorio.c -o aleatorio.asm

simulador_completo/bin/montador compilador_c/rt/runtime.asm compilador_c/rt/biblioteca.asm aleatorio.asm -o aleatorio.mob

simulador_completo/bin/simulador aleatorio.mob
