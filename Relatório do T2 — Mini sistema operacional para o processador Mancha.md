# Relatório do T2 — Mini sistema operacional para o processador Mancha

**Disciplina:** Sistemas Operacionais — UFSM **Autores:** Gustavo Conrad e João Pedro Guerra

## 1. O sistema implementado

O SO roda sobre a BIOS fornecida (`bios_mancha`) e é escrito em C (compilado pelo `mcc`), com uma pequena ponte em assembly para o que C não expressa (portas de E/S, `halt`, instalação das chamadas de sistema e do relógio, primeiro `rete`).

| Arquivo     | Conteúdo                                                                                               |
| ----------- | ------------------------------------------------------------------------------------------------------ |
| `so.c`      | kernel: tabela de processos, escalonadores, chamadas de sistema, métricas e relatório                  |
| `so.h`      | números das chamadas de sistema e configuração (`ESCALONADOR`, `QUANTUM`, `TICKS`)                     |
| `ponte.asm` | E/S, instalação das syscalls (`trap 7`), relógio, contagem de interrupções de console, primeiro `rete` |
| `usuario.c` | processos de usuário e biblioteca de chamadas de sistema                                               |
| `ulib.asm`  | rotina `so_chama`, que executa o `trap 7`                                                              |
| `Makefile`  | compila, monta e executa                                                                               |

**Processos.** Tabela de 8 entradas (`struct Processo`), com estados PRONTO, EXECUTANDO e BLOQUEADO (e LIVRE para entrada vazia). O pid é um número de série que não é reaproveitado (até 16 processos por execução), enquanto a entrada da tabela é reutilizada quando o processo morre. Cada processo guarda um quadro de 16 palavras, copiado de e para o endereço fixo onde a CPU empilha o contexto a cada interrupção (`salva_quadro` e `restaura_quadro`). Todos começam em modo usuário (`sr = 0`, ou seja, S=0 e I=0): qualquer `in`/`out`/`halt` direto causaria a interrupção 2. As métricas de cada processo ficam em um vetor separado (`est[]`, indexado por pid − 1), para sobreviverem à reutilização da entrada da tabela.

**Chamadas de sistema** (`trap 7`, número em `r0`, argumentos em `r1`..): `SO_LE`, `SO_ESCREVE`, `SO_CRIA_PROC`, `SO_MATA_PROC` (pid 0 = o próprio processo) e `SO_ESPERA_PROC`. `SO_LE` e `SO_ESCREVE` bloqueiam o processo quando o dispositivo não está pronto; `SO_ESPERA_PROC` bloqueia até a morte do processo indicado e devolve erro se ele não existe ou é o próprio chamador. Quando um processo morre, `so_mata` chama `acorda_esperando`, que desbloqueia quem esperava por ele.

**Bloqueio e pendências.** A função `so_pendencias` roda a cada interrupção do relógio e a cada troca de processo: verifica os dispositivos que causaram bloqueio, faz a operação pendente e desbloqueia o processo. Quando todos os processos estão bloqueados, roda um processo ocioso em modo usuário, com as interrupções ligadas.

**Escalonadores** (escolhidos em `so.h`):

- **Round-robin:** fila de prontos; quem fica pronto (criado ou desbloqueado) vai para o fim; o escalonador escolhe o primeiro. O quantum é medido em interrupções de relógio (tiques); ao zerar, o processo vai para o fim da fila se houver outro pronto.
- **Prioridade:** mesmo mecanismo de quantum, mas escolhe o pronto de menor valor de prioridade (empate: o que está há mais tempo na fila). A prioridade começa em 0,5 e, quando o processo perde a CPU (bloqueio ou fim de quantum), vira `prio = (prio + t_exec / t_quantum) / 2`, com `t_exec = quantum − quantum restante`. Implementada em ponto fixo (×1000).

**Métricas** (impressas quando o init morre): processos criados, tempo total, tempo ocioso, interrupções por tipo, preempções, e por processo: tempo de retorno, preempções, número de entradas e tempo em cada estado e tempo médio de resposta (entre desbloquear e ser escalonado). Quando um processo nunca foi desbloqueado, a resposta média é impressa como `0`; quando foi desbloqueado e escalonado de imediato, como `0.0`.

## 2. Metodologia

**Carga de trabalho** (`usuario.c`). O init (pid 1) cria quatro processos e espera cada um morrer:

| pid | Processo | Comportamento                                                                 |
| --- | -------- | ----------------------------------------------------------------------------- |
| 2   | A        | 6 rodadas de cálculo (laço de 400 voltas) seguidas de `SO_ESCREVE`            |
| 3   | B        | idem, 6 rodadas                                                               |
| 4   | C        | idem, 2 rodadas (processo curto)                                              |
| 5   | pai      | cria o filho, faz 1 rodada, depois bloqueia em `SO_ESPERA_PROC` e escreve `p` |
| 6   | filho    | 3 rodadas de cálculo e escrita                                                |

**Configurações testadas:** round-robin e prioridade, cada um com quantum de 1, 2 e 4 tiques (6 execuções). Um tique é uma interrupção de relógio, programada a cada 5000 instruções.

**Unidade de tempo.** Todos os tempos estão em **tiques** (1 tique = 5000 instruções, medidas pelo contador de interrupções do relógio). Eventos que ocorrem entre dois tiques não são distinguidos. Além disso, a tabela 3.1 informa o total de instruções simuladas, lido do próprio simulador.

**Coleta.** Cada configuração foi executada uma vez no simulador (`D0`, `C`), saindo do simulador com `F` antes da seguinte (o comando `R` do simulador reinicia a CPU mas não recarrega a memória, o que acumularia as métricas). Como a janela da console do simulador mostra só as últimas 7 linhas, a saída do terminal foi gravada com o comando `script` e o relatório foi lido dessa gravação. A execução Prioridade q4 foi feita duas vezes (a primeira gravação ficou incompleta): os valores comuns às duas coincidem (47 tiques, 9 preempções e os pids 1 a 4), o que confirma que a simulação é determinística.

## 3. Resultados

### 3.1 Totais

| Configuração          | Tempo total (tiques) | Tempo ocioso | Preempções | Interrupções de relógio | Instruções simuladas |
| --------------------- | -------------------- | ------------ | ---------- | ----------------------- | -------------------- |
| RR, quantum 1         | 74                   | 0            | 71         | 74                      | 408401               |
| RR, quantum 2         | 52                   | 0            | 22         | 52                      | 298025               |
| RR, quantum 4         | 46                   | 0            | 9          | 46                      | 271294               |
| Prioridade, quantum 1 | 81                   | 0            | 76         | 81                      | 445001               |
| Prioridade, quantum 2 | 54                   | 0            | 23         | 54                      | 309234               |
| Prioridade, quantum 4 | 47                   | 0            | 9          | 47                      | 274761               |

Em todas as execuções foram criados 6 processos, houve 0 interrupções de console (ninguém digitou) e o número de chamadas de sistema (`trap 7`) foi **36**. Esse valor não depende do escalonador nem do quantum: é a soma das chamadas feitas por `usuario.c` (init 10, A 7, B 7, C 3, pai 5, filho 4).

Observação sobre as duas últimas colunas: o total de instruções é maior que tiques × 5000 por uma diferença quase constante, entre 38 mil e 41 mil instruções em todas as execuções (por exemplo, RR q1: 408401 − 74 × 5000 = 38401; Prioridade q4: 274761 − 47 × 5000 = 39761). Isso é compatível com o custo de imprimir o relatório e das instruções de inicialização, que ocorrem fora da janela medida pelo relógio (o tempo total é lido no início de `so_relatorio`, antes de imprimir).

### 3.2 Tempo de retorno por processo (tiques)

| Processo     | RR q1 | RR q2 | RR q4 | Prio q1 | Prio q2 | Prio q4 |
| ------------ | ----- | ----- | ----- | ------- | ------- | ------- |
| 1 (init)     | 74    | 52    | 46    | 81      | 54      | 47      |
| 2 (A)        | 73    | 51    | 42    | 80      | 52      | 43      |
| 3 (B)        | 71    | 48    | 44    | 78      | 53      | 46      |
| 4 (C, curto) | 33    | 23    | 23    | 40      | 25      | 28      |
| 5 (pai)      | 52    | 44    | 45    | 55      | 38      | 40      |
| 6 (filho)    | 47    | 34    | 31    | 50      | 31      | 27      |

### 3.3 Tempo médio de resposta do pai (tiques)

O pai é o único processo (além do init) que bloqueia e é desbloqueado durante a execução, então é o que tem resposta média relevante. O init também é desbloqueado, mas sua resposta média foi 0.0 em todas as execuções onde foi lida.

| RR q1 | RR q2 | RR q4 | Prio q1 | Prio q2 | Prio q4 |
| ----- | ----- | ----- | ------- | ------- | ------- |
| 2.0   | 4.0   | 1.0   | 0.0     | 0.0     | 0.0     |

### 3.4 Estados (pronto / bloqueado / executando)

Para cada processo, pronto + bloqueado + executando é igual ao tempo de retorno, e a soma do tempo em estado executando de todos os processos é igual ao tempo total (a CPU nunca ficou ociosa). Isso serve de verificação das métricas. Exemplo completo, Prioridade com quantum 4 (formato: número de entradas no estado / tiques no estado):

| pid | Pronto | Bloqueado | Executando | Soma (= retorno) |
| --- | ------ | --------- | ---------- | ---------------- |
| 1   | 3x / 0 | 2x / 44   | 3x / 3     | 47               |
| 2   | 4x / 29 | 0x / 0   | 4x / 14    | 43               |
| 3   | 4x / 32 | 0x / 0   | 4x / 14    | 46               |
| 4   | 2x / 23 | 0x / 0   | 2x / 5     | 28               |
| 5   | 2x / 12 | 1x / 25  | 2x / 3     | 40               |
| 6   | 3x / 19 | 0x / 0   | 3x / 8     | 27               |

Soma do tempo executando: 3 + 14 + 14 + 5 + 3 + 8 = 47 tiques, igual ao tempo total. Soma das preempções por processo: 0 + 3 + 3 + 1 + 0 + 2 = 9, igual ao total.

## 4. Análise

1. **Quantum maior reduz o custo de troca de contexto.** Nos dois escalonadores o tempo total cai com o quantum (RR: 74, 52, 46; prioridade: 81, 54, 47) e as preempções despencam (RR: 71, 22, 9). Cada troca copia dois quadros de 16 palavras e percorre a fila em C sem otimização, e esse tempo conta no relógio; com quantum 1 há uma troca por tique. O ganho diminui a cada passo (de 2 para 4 o total cai pouco).
2. **O escalonador por prioridade custou mais que o round-robin nesta carga:** 81 contra 74, 54 contra 52 e 47 contra 46 tiques. Em instruções simuladas a diferença é de 9% com quantum 1 (445001 contra 408401), 3,8% com quantum 2 e 1,3% com quantum 4. Provavelmente vem de duas fontes: as preempções a mais (76 contra 71 e 23 contra 22; com quantum 4 são 9 nos dois) e o custo de varrer a fila atrás do menor valor de prioridade a cada escolha.
3. **Prioridade favorece quem bloqueia.** O pai, que passa a maior parte do tempo bloqueado, ficou com resposta média 0.0 nos três quanta (contra 2.0, 4.0 e 1.0 no round-robin) e terminou antes com quantum 2 (38 contra 44 tiques) e com quantum 4 (40 contra 45). Com quantum 1 esse ganho não aparece no retorno (55 contra 52). O filho, criado depois de A e B já terem rodado, também terminou antes com quantum 2 (31 contra 34) e 4 (27 contra 31), mas não com quantum 1 (50 contra 47). Uma explicação provável (não verificada por experimento separado) é que o filho nasce com prioridade 0,5, melhor que a de A e B, que já gastaram quantum inteiro. Com quantum 1, `t_exec / t_quantum` só vale 0 ou 1, e a política perde resolução.
4. **Prioridade não favorece processos curtos.** O processo C (curto, só calcula) terminou depois em prioridade que em round-robin (40 contra 33 com quantum 1, 25 contra 23 com quantum 2, 28 contra 23 com quantum 4), porque a política só olha a fração do quantum usada, e C usa o quantum inteiro, como A e B.
5. **Resposta média e granularidade.** A resposta média do pai no round-robin (2.0, 4.0, 1.0) não segue tendência com o quantum. Com tique de 5000 instruções, diferenças de um ou dois tiques estão no limite da resolução da medida, e este trabalho não tira conclusão forte desses valores.

## 5. Limitações

- **Resolução:** tempos em tiques de 5000 instruções. Quantum de 1 tique significa que `t_exec / t_quantum` só assume valores como 0 ou 1.
- **Uma execução por configuração:** a simulação é determinística (a repetição da execução Prioridade q4 deu os mesmos valores), mas não foram feitas execuções com cargas diferentes.
- **Carga única:** os resultados valem para a carga descrita na seção 2. Essa carga não exercita todos os caminhos do SO nas métricas: ninguém digitou, então `SO_LE` não bloqueou (0 interrupções de console), e o tempo ocioso foi 0 em todas as execuções, porque sempre havia processo pronto. O processo ocioso e o bloqueio por leitura estão implementados (`so_troca`, `so_pendencias`), mas não aparecem nos números deste relatório.
- **Total de instruções × tiques:** o total de instruções do simulador excede tiques × 5000 por cerca de 38 a 41 mil instruções, compatível com o custo de imprimir o relatório e da inicialização, que ficam fora da janela medida pelo relógio (ver seção 3.1). Por isso as conclusões usam só comparações relativas entre tempos em tiques.
- **Violações:** uma exceção de CPU (violação de segmento, instrução privilegiada) cai no pânico da BIOS e para o sistema; não há tratamento que mate só o processo culpado.

## 6. Como reproduzir

Com o repositório do professor clonado e compilado (`make` na raiz), coloque os arquivos desta entrega numa pasta ao lado de `bios_mancha/` e, nessa pasta:

```
make clean
make
make run
```

No simulador: `D0`, `C`, esperar `CPU parada (halt)` e sair com `F`. Para trocar de configuração, edite `ESCALONADOR` (1 = round-robin, 2 = prioridade) e `QUANTUM` em `so.h` e rode `make clean && make run` de novo. Para gravar a saída, rode `script -q saida.txt` antes do `make run` e `exit` depois de sair do simulador.
