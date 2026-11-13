; ulib.asm - int so_chama(int num, int a1, int a2): faz "trap 7" (convencao do mcc)
        .text
_f_so_chama:
        push    bp
        ld      bp, sp
        ld      r0, (bp+4)
        ld      r1, (bp+6)
        ld      r2, (bp+8)
        trap    7
        ld      sp, bp
        pop     bp
        ret
