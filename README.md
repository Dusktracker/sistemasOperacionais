# T1 — Sistemas Operacionais — Processador Mancha

**Autores:** João Guerra (202512648) e Gustavo Ribeiro (202512653)

## Descrição

Programa que gera um número pseudoaleatório, escrito no subconjunto de C
do compilador `mcc` para o processador didático Mancha.

## Estrutura esperada

Este `aleatorio.c` deve ser colocado na **raiz** do repositório do
Mancha (https://gitlab.inf.ufsm.br/jvlima/mancha), ao lado das pastas
`compilador_c/` e `simulador_completo/`:

```
mancha/
├── aleatorio.c        <- este arquivo
├── compilador_c/
├── simulador_completo/
└── Makefile
```

## Como compilar e rodar

```sh
make

compilador_c/bin/mcc -Icompilador_c/inc aleatorio.c -o aleatorio.asm

simulador_completo/bin/montador compilador_c/rt/runtime.asm compilador_c/rt/biblioteca.asm aleatorio.asm -o aleatorio.mob

simulador_completo/bin/simulador aleatorio.mob
```

O programa não precisa de nenhuma interação do operador: ele lê o
relógio do simulador, calcula o número e imprime sozinho, parando
(`halt`) em seguida. O resultado aparece no painel **"console (saída)"**
da interface.

## Como funciona a geração do número

O programa lê o contador do relógio do simulador (porta `0020`) como
semente e aplica sobre ela uma combinação de multiplicação e soma por
constantes primas, seguida de valor absoluto e módulo 100, para
produzir um número pseudoaleatório entre 0 e 99:

```
resultado = |semente * 31337 + 7919| % 100
```

## Arquivos

| Arquivo | Descrição |
|---|---|
| `aleatorio.c` | código-fonte |
| `aleatorio.asm` | assembly Mancha gerado pelo `mcc` a partir do `.c` |
