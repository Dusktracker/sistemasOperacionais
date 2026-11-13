/* so.c - T2: processos, escalonadores (round-robin e por prioridade) com
 * preempcao, bloqueio por E/S e por espera de processo, processo ocioso,
 * chamadas de sistema e metricas.
 * O que C nao expressa (portas, halt, rete, trap) fica em ponte.asm.
 * Tempos medidos em TIQUES do relogio (1 tique = TICKS instrucoes). */
#include "so.h"

#define MAX_PROC   8
#define MAX_PID    16      /* maximo de processos criados durante a execucao */
#define LIVRE      0
#define PRONTO     1
#define EXECUTANDO 2
#define BLOQUEADO  3

#define ESPERA_LE      1
#define ESPERA_ESCREVE 2
#define ESPERA_PROC    3

struct Processo {
    int estado;
    int pid;
    int motivo;    /* por que esta bloqueado */
    int arg;       /* caractere a escrever / pid esperado */
};

/* metricas de cada processo, indexadas por (pid - 1): sobrevivem a reutilizacao da entrada */
struct Estat {
    int criacao;
    int fim;
    int preempcoes;
    int n_pronto;
    int n_bloq;
    int n_exec;
    int t_pronto;
    int t_bloq;
    int t_exec;
    int desde;         /* instante da ultima mudanca de estado */
    int resp_soma;     /* soma dos tempos entre desbloquear e ser escalonado */
    int resp_n;
    int resp_pend;     /* 1 = acabou de desbloquear, falta ser escalonado */
    int prio;          /* prioridade x1000 (500 = 0,5): menor valor = maior prioridade */
};

struct Processo tabela[MAX_PROC];
struct Estat est[MAX_PID];
int quadros[128];                   /* quadro de 16 palavras por processo: 8 * 16 (literal, exigido pelo mcc) */
int pilhas[MAX_PROC][64];
int pilha_ocioso[32];
int atual;                          /* indice do processo em execucao, -1 = nenhum */
int proximo_pid;                    /* numero de serie: nunca e reutilizado */
int fila[MAX_PROC];                 /* fila de prontos (indices da tabela) */
int nf;
int quantum_restante;
int ticks_por_fatia = TICKS;        /* lido por ponte.asm para programar o relogio */

/* metricas globais */
int ints[16];                       /* interrupcoes por tipo (7 = trap, 10 = relogio) */
int n_preempcoes;
int ocioso_total;
int ocioso_ativo;
int ocioso_desde;

/* implementadas em ponte.asm */
void escreve_caractere(int c);
int saida_pronta(void);
int le_console(void);
int le_tiques(void);
int le_cnt_console(void);
void so_halt(void);

/* implementadas em usuario.c */
void proc_init(void);
void fim_processo(void);

void so_imprime(char *s)
{
    while (*s != 0) {
        escreve_caractere(*s);
        s = s + 1;
    }
}

void so_imprime_int(int v)
{
    if (v < 0) {
        escreve_caractere('-');
        v = 0 - v;
    }
    if (v >= 10) {
        so_imprime_int(v / 10);
    }
    escreve_caractere('0' + v % 10);
}

void linha(char *rotulo, int v)
{
    so_imprime(rotulo);
    so_imprime_int(v);
    escreve_caractere('\n');
}

/* ------------------------------------------------------------ metricas */

/* troca o estado do processo i contabilizando tempo e numero de entradas */
void muda_estado(int i, int novo)
{
    int e;
    int agora;
    int ant;
    e = tabela[i].pid - 1;
    agora = le_tiques();
    ant = tabela[i].estado;
    if (ant == PRONTO) {
        est[e].t_pronto = est[e].t_pronto + agora - est[e].desde;
    }
    if (ant == EXECUTANDO) {
        est[e].t_exec = est[e].t_exec + agora - est[e].desde;
    }
    if (ant == BLOQUEADO) {
        est[e].t_bloq = est[e].t_bloq + agora - est[e].desde;
    }
    if (novo == EXECUTANDO && est[e].resp_pend == 1) {
        est[e].resp_soma = est[e].resp_soma + agora - est[e].desde;
        est[e].resp_n = est[e].resp_n + 1;
        est[e].resp_pend = 0;
    }
    est[e].desde = agora;
    tabela[i].estado = novo;
    if (novo == PRONTO) {
        est[e].n_pronto = est[e].n_pronto + 1;
    }
    if (novo == BLOQUEADO) {
        est[e].n_bloq = est[e].n_bloq + 1;
    }
    if (novo == EXECUTANDO) {
        est[e].n_exec = est[e].n_exec + 1;
    }
    if (novo == LIVRE) {
        est[e].fim = agora;
    }
}

/* prio = (prio + t_exec / t_quantum) / 2, em ponto fixo (x1000) */
void atualiza_prio(int i)
{
    int e;
    int usado;
    e = tabela[i].pid - 1;
    usado = QUANTUM - quantum_restante;
    est[e].prio = (est[e].prio + usado * 1000 / QUANTUM) / 2;
}

void imprime_media(int soma, int n)
{
    int v;
    if (n == 0) {
        so_imprime_int(0);
        return;
    }
    v = soma * 10 / n;
    so_imprime_int(v / 10);
    escreve_caractere('.');
    so_imprime_int(v % 10);
}

void so_relatorio(void)
{
    int p;
    int total;
    total = le_tiques();
    if (ocioso_ativo == 1) {
        ocioso_total = ocioso_total + total - ocioso_desde;
        ocioso_ativo = 0;
    }
    so_imprime("\n==== RELATORIO ====\n");
    linha("escalonador (1=RR, 2=PRIO): ", ESCALONADOR);
    linha("quantum (tiques): ", QUANTUM);
    linha("instrucoes por tique: ", ticks_por_fatia);
    linha("processos criados: ", proximo_pid - 1);
    linha("tempo total de execucao (tiques): ", total);
    linha("tempo ocioso (tiques): ", ocioso_total);
    linha("interrupcoes de relogio (10): ", ints[10]);
    linha("interrupcoes de console (8): ", le_cnt_console());
    linha("chamadas de sistema, trap 7: ", ints[7]);
    linha("preempcoes: ", n_preempcoes);
    for (p = 0; p < proximo_pid - 1; p++) {
        so_imprime("-- pid ");
        so_imprime_int(p + 1);
        so_imprime(": retorno=");
        so_imprime_int(est[p].fim - est[p].criacao);
        so_imprime(" preempcoes=");
        so_imprime_int(est[p].preempcoes);
        so_imprime(" resposta_media=");
        imprime_media(est[p].resp_soma, est[p].resp_n);
        so_imprime("\n   pronto: ");
        so_imprime_int(est[p].n_pronto);
        so_imprime("x/");
        so_imprime_int(est[p].t_pronto);
        so_imprime("  bloqueado: ");
        so_imprime_int(est[p].n_bloq);
        so_imprime("x/");
        so_imprime_int(est[p].t_bloq);
        so_imprime("  executando: ");
        so_imprime_int(est[p].n_exec);
        so_imprime("x/");
        so_imprime_int(est[p].t_exec);
        escreve_caractere('\n');
    }
}

void so_fim(void)
{
    so_relatorio();
    so_halt();
}

/* ------------------------------------------------------------- processos */

void ocioso(void)
{
    for (;;) {
    }
}

void instala_ocioso(int *quadro)
{
    int k;
    for (k = 0; k < 16; k++) {
        quadro[k] = 0;
    }
    quadro[6] = (int) &pilha_ocioso[31];
    quadro[7] = (int) ocioso;
    quadro[8] = 0;
}

void monta_processo(int i, int ender)
{
    int base;
    int k;
    base = i * 16;
    for (k = 0; k < 16; k++) {
        quadros[base + k] = 0;
    }
    pilhas[i][63] = (int) fim_processo;
    quadros[base + 6] = (int) &pilhas[i][63];
    quadros[base + 7] = ender;
    quadros[base + 8] = 0;               /* sr: modo usuario */
}

void fila_entra(int i)
{
    fila[nf] = i;
    nf = nf + 1;
}

void fila_remove(int i)
{
    int k;
    int j;
    for (k = 0; k < nf; k++) {
        if (fila[k] == i) {
            for (j = k; j < nf - 1; j++) {
                fila[j] = fila[j + 1];
            }
            nf = nf - 1;
            return;
        }
    }
}

int so_cria(int ender)
{
    int i;
    int e;
    if (proximo_pid > MAX_PID) {
        return -1;
    }
    for (i = 0; i < MAX_PROC; i++) {
        if (tabela[i].estado == LIVRE) {
            monta_processo(i, ender);
            tabela[i].pid = proximo_pid;
            tabela[i].motivo = 0;
            tabela[i].arg = 0;
            e = proximo_pid - 1;
            est[e].criacao = le_tiques();
            est[e].desde = est[e].criacao;
            est[e].prio = 500;
            proximo_pid = proximo_pid + 1;
            muda_estado(i, PRONTO);
            fila_entra(i);
            return tabela[i].pid;
        }
    }
    return -1;
}

int busca_pid(int pid)
{
    int i;
    for (i = 0; i < MAX_PROC; i++) {
        if (tabela[i].estado != LIVRE && tabela[i].pid == pid) {
            return i;
        }
    }
    return -1;
}

int ha_vivos(void)
{
    int i;
    for (i = 0; i < MAX_PROC; i++) {
        if (tabela[i].estado != LIVRE) {
            return 1;
        }
    }
    return 0;
}

void salva_quadro(int *quadro)
{
    int base;
    int k;
    if (atual < 0) {
        return;
    }
    base = atual * 16;
    for (k = 0; k < 16; k++) {
        quadros[base + k] = quadro[k];
    }
}

void restaura_quadro(int *quadro)
{
    int base;
    int k;
    if (atual < 0) {
        return;
    }
    base = atual * 16;
    for (k = 0; k < 16; k++) {
        quadro[k] = quadros[base + k];
    }
}

int prio_de(int i)
{
    return est[tabela[i].pid - 1].prio;
}

/* escolhe o proximo processo: RR = primeiro da fila; PRIO = menor valor de
 * prioridade entre os prontos (empate: o que esta ha mais tempo na fila) */
void so_escalona(void)
{
    int k;
    int melhor;
    int i;
    int agora;
    if (atual >= 0 && tabela[atual].estado == EXECUTANDO) {
        return;
    }
    atual = -1;
    if (nf == 0) {
        return;
    }
    melhor = 0;
    if (ESCALONADOR == 2) {
        for (k = 1; k < nf; k++) {
            if (prio_de(fila[k]) < prio_de(fila[melhor])) {
                melhor = k;
            }
        }
    }
    i = fila[melhor];
    for (k = melhor; k < nf - 1; k++) {
        fila[k] = fila[k + 1];
    }
    nf = nf - 1;
    if (ocioso_ativo == 1) {
        agora = le_tiques();
        ocioso_total = ocioso_total + agora - ocioso_desde;
        ocioso_ativo = 0;
    }
    atual = i;
    quantum_restante = QUANTUM;
    muda_estado(i, EXECUTANDO);
}

void desbloqueia(int k, int ret)
{
    quadros[k * 16] = ret;                 /* r0 do quadro salvo = valor de retorno da chamada */
    muda_estado(k, PRONTO);
    est[tabela[k].pid - 1].resp_pend = 1;
    fila_entra(k);
}

void acorda_esperando(int pid)
{
    int k;
    for (k = 0; k < MAX_PROC; k++) {
        if (tabela[k].estado == BLOQUEADO && tabela[k].motivo == ESPERA_PROC
            && tabela[k].arg == pid) {
            desbloqueia(k, 0);
        }
    }
}

/* trata pendencias: dispositivos que causaram bloqueio */
void so_pendencias(void)
{
    int k;
    int c;
    for (k = 0; k < MAX_PROC; k++) {
        if (tabela[k].estado == BLOQUEADO) {
            if (tabela[k].motivo == ESPERA_LE) {
                c = le_console();
                if (c >= 0) {
                    desbloqueia(k, c);
                }
            } else if (tabela[k].motivo == ESPERA_ESCREVE) {
                if (saida_pronta()) {
                    escreve_caractere(tabela[k].arg);
                    desbloqueia(k, 0);
                }
            }
        }
    }
}

/* tira o processo atual da CPU e coloca outro (ou o ocioso).
 * Devolve o r0 do quadro instalado (a BIOS grava o retorno do handler em r0). */
int so_troca(int *quadro, int morto)
{
    if (morto == 0) {
        salva_quadro(quadro);
    }
    so_pendencias();
    so_escalona();
    if (atual < 0) {
        if (ha_vivos()) {
            if (ocioso_ativo == 0) {
                ocioso_ativo = 1;
                ocioso_desde = le_tiques();
            }
            instala_ocioso(quadro);
            return 0;
        }
        so_fim();
    }
    restaura_quadro(quadro);
    return quadro[0];
}

int bloqueia(int *quadro, int motivo, int arg)
{
    atualiza_prio(atual);
    muda_estado(atual, BLOQUEADO);
    tabela[atual].motivo = motivo;
    tabela[atual].arg = arg;
    return so_troca(quadro, 0);
}

int so_mata(int *quadro, int pid)
{
    int i;
    i = atual;
    if (pid != 0) {
        i = busca_pid(pid);
    }
    if (i < 0) {
        return -1;
    }
    if (tabela[i].estado == PRONTO) {
        fila_remove(i);
    }
    acorda_esperando(tabela[i].pid);
    muda_estado(i, LIVRE);
    if (tabela[i].pid == 1) {
        so_fim();                 /* init morreu: fim do SO */
    }
    if (i != atual) {
        return 0;
    }
    return so_troca(quadro, 1);
}

int so_syscall(int *quadro, int num, int a1, int a2)
{
    int i;
    int c;
    ints[7] = ints[7] + 1;
    if (num == SO_LE) {
        c = le_console();
        if (c >= 0) {
            return c;
        }
        return bloqueia(quadro, ESPERA_LE, 0);
    }
    if (num == SO_ESCREVE) {
        if (saida_pronta()) {
            escreve_caractere(a1);
            return 0;
        }
        return bloqueia(quadro, ESPERA_ESCREVE, a1);
    }
    if (num == SO_CRIA_PROC) {
        return so_cria(a1);
    }
    if (num == SO_MATA_PROC) {
        return so_mata(quadro, a1);
    }
    if (num == SO_ESPERA_PROC) {
        i = busca_pid(a1);
        if (i < 0 || i == atual) {
            return -1;
        }
        return bloqueia(quadro, ESPERA_PROC, a1);
    }
    return -1;
}

/* interrupcao do relogio: trata pendencias e aplica a preempcao por quantum */
void so_tick(int *quadro)
{
    int antigo;
    ints[10] = ints[10] + 1;
    so_pendencias();
    if (atual < 0) {
        so_escalona();
        if (atual >= 0) {
            restaura_quadro(quadro);
        }
        return;
    }
    quantum_restante = quantum_restante - 1;
    if (quantum_restante > 0) {
        return;
    }
    atualiza_prio(atual);
    if (nf == 0) {
        quantum_restante = QUANTUM;      /* ninguem esperando: continua */
        return;
    }
    antigo = atual;
    salva_quadro(quadro);
    muda_estado(atual, PRONTO);
    fila_entra(atual);
    so_escalona();
    restaura_quadro(quadro);
    if (atual != antigo) {
        n_preempcoes = n_preempcoes + 1;
        est[tabela[antigo].pid - 1].preempcoes = est[tabela[antigo].pid - 1].preempcoes + 1;
    }
}

void so_inicia(int *quadro_boot)
{
    proximo_pid = 1;
    atual = -1;
    nf = 0;
    so_cria((int) proc_init);
    so_escalona();
    restaura_quadro(quadro_boot);
}
