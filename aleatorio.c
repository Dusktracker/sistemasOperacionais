#include "mancha.h"

int main(void)
{
    int semente;
    int resultado;

    semente = mancha_in(32);

    resultado = (semente * 31337) + 7919;
    if (resultado < 0) {
        resultado = resultado * -1;
    }
    resultado = resultado % 100;

    puts("Numero gerado:");
    print_int(resultado);
    putchar('\n');

    mancha_halt();
    return 0;
}
