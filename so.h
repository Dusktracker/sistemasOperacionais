/* so.h - numeros das chamadas de sistema e configuracao do SO */
#define SO_LE          1
#define SO_ESCREVE     2
#define SO_CRIA_PROC   3
#define SO_MATA_PROC   4
#define SO_ESPERA_PROC 5

/* ---- configuracao (mude aqui para gerar o relatorio em varias configuracoes) ---- */
#define ESCALONADOR 2     /* 1 = round-robin, 2 = prioridade */
#define QUANTUM     2     /* em interrupcoes do relogio (tiques) */
#define TICKS       5000  /* instrucoes entre duas interrupcoes do relogio (1 tique) */
