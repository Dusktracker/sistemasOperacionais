# T2 — Sistemas Operacionais — Processador Mancha

**Autores:** João Guerra (202512648) e Gustavo Ribeiro (202512653)

## Descrição

Núcleo de Sistema Operacional multitarefa preemptivo com escalonamento por
prioridade dinâmica, escrito para o processador didático Mancha. O kernel
gerencia múltiplos processos concorrentes, chamadas de sistema via `TRAP 7`,
entrada/saída assíncrona com bloqueio e coleta de métricas de execução.

## Estrutura esperada

Os arquivos deste trabalho devem ser colocados em uma pasta (ex: `t2/`)
dentro da raiz do repositório do Mancha (https://gitlab.inf.ufsm.br/jvlima/mancha),
ao lado das pastas `compilador_c/` e `simulador_completo/`:

```
mancha/
├── compilador_c/
├── simulador_completo/
└── t2/
    ├── Makefile
    ├── ponte.asm
    ├── so.c
    ├── so.h
    ├── ulib.asm
    └── usuario.c
```

## Como compilar e rodar

Dentro da pasta do trabalho, compile e execute no simulador:

```sh
make

../simulador_completo/bin/simulador so.mob
```

O sistema inicia automaticamente o processo `proc_init`, que cria os
processos concorrentes de teste (`proc_a`, `proc_b`, `proc_c` e `proc_pai`,
que cria `proc_filho`). A saída dos caracteres aparece no painel **"console
(saída)"** e, ao final da execução, o relatório de estatísticas é exibido.

## Como funciona o Sistema Operacional

* **Chamadas de Sistema (`TRAP 7`):** Programas de usuário invocam serviços
  do kernel (`SO_LE`, `SO_ESCREVE`, `SO_CRIA_PROC`, `SO_MATA_PROC`,
  `SO_ESPERA_PROC`) através de `ulib.asm`.
* **Troca de Contexto (`ponte.asm`):** Salva e restaura os registradores
  (`PC`, `A`, `X`, `Y`) na tabela de processos (`MAX_PROC = 16`) a cada
  interrupção de relógio ou `TRAP 7`.
* **Bloqueio de E/S:** Processos que aguardam o terminal ou outro processo
  (`espera_proc`) mudam para o estado `BLOQUEADO`, liberando a CPU. Se todos
  estiverem bloqueados, a CPU executa `HALT` até a próxima interrupção.
* **Escalonador com Prioridade Dinâmica:** Com quantum de 2 ticks
  (`QUANTUM_MAX = 2`), cada processo inicia com prioridade `0.5` (menor valor =
  maior prioridade) e tem sua prioridade atualizada a cada parada pela fórmula:

```
prioridade = (prioridade + (tempo_usado / QUANTUM_MAX)) / 2
```

## Arquivos

| Arquivo | Descrição |
|---|---|
| `Makefile` | regras de compilação e montagem para gerar `so.mob` |
| `so.h` | definições da tabela de processos, estados, syscalls e constantes |
| `so.c` | implementação do kernel, interrupções, E/S, escalonador e métricas |
| `ponte.asm` | salvamento e restauração de contexto nas interrupções |
| `ulib.asm` | interface em assembly (`so_chama`) que executa `TRAP 7` |
| `usuario.c` | processos de teste em modo usuário |
