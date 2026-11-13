/* usuario.c - programas que rodam em modo usuario (sem in/out/halt) */
#include "so.h"

int so_chama(int num, int a1, int a2);   /* ulib.asm: faz o trap 7 */

void escreve(int c)
{
    so_chama(SO_ESCREVE, c, 0);
}

int le(void)
{
    return so_chama(SO_LE, 0, 0);
}

int cria_proc(int endereco)
{
    return so_chama(SO_CRIA_PROC, endereco, 0);
}

int mata_proc(int pid)
{
    return so_chama(SO_MATA_PROC, pid, 0);     /* pid 0 = o proprio processo */
}

int espera_proc(int pid)
{
    return so_chama(SO_ESPERA_PROC, pid, 0);
}

/* o kernel coloca o endereco desta funcao como endereco de retorno de todo processo */
void fim_processo(void)
{
    mata_proc(0);
}

/* gasta CPU (cerca de 25 instrucoes por volta) */
void calcula(int n)
{
    int i;
    int x;
    x = 0;
    for (i = 0; i < n; i++) {
        x = x + i;
    }
}

/* alterna "calcula" e escreve a letra c, "rodadas" vezes */
void trabalha(int c, int rodadas)
{
    int i;
    for (i = 0; i < rodadas; i++) {
        calcula(400);
        escreve(c);
    }
}

void proc_a(void)
{
    trabalha('A', 6);
}

void proc_b(void)
{
    trabalha('B', 6);
}

void proc_c(void)
{
    trabalha('C', 2);
}

void proc_filho(void)
{
    trabalha('f', 3);
}

/* cria um filho e espera por ele: bloqueia por SO_ESPERA_PROC */
void proc_pai(void)
{
    int p;
    p = cria_proc((int) proc_filho);
    trabalha('P', 1);
    espera_proc(p);
    escreve('p');
}

void proc_init(void)
{
    int pa;
    int pb;
    int pc;
    int pp;
    pa = cria_proc((int) proc_a);
    pb = cria_proc((int) proc_b);
    pc = cria_proc((int) proc_c);
    pp = cria_proc((int) proc_pai);
    espera_proc(pa);
    espera_proc(pb);
    espera_proc(pc);
    espera_proc(pp);
    escreve('\n');
}
