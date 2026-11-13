; ponte.asm - o que C nao expoe: portas, halt, instalacao das syscalls e do relogio,
; contagem de interrupcoes do console, primeiro rete.
        .text

; void escreve_caractere(int c)
_f_escreve_caractere:
        push    bp
        ld      bp, sp
        ld      r0, (bp+4)
        outb    r0, (1)
        ld      sp, bp
        pop     bp
        ret

; void so_halt(void)
_f_so_halt:
        halt
        ret

; int saida_pronta(void): 1 se a console aceita um caractere agora (bit 0 da porta 2)
_f_saida_pronta:
        inb     r0, (2)
        and     r0, 1
        ret

; int le_console(void): proximo caractere do buffer da BIOS, ou -1 se vazio
_f_le_console:
        push    bp
        ld      bp, sp
        call    bios_console_disponivel
        cmp     r0, 0
        jmpc    eq, _pt_vazio
        call    bios_console_le
        jmp     _pt_lc_fim
_pt_vazio:
        ld      r0, -1
_pt_lc_fim:
        ld      sp, bp
        pop     bp
        ret

; int le_tiques(void): numero de interrupcoes de relogio ate agora (contador da BIOS)
_f_le_tiques:
        ld      r0, (contador_tiques)
        ret

; int le_cnt_console(void): numero de interrupcoes de console recebidas
_f_le_cnt_console:
        ld      r0, (_cnt_console)
        ret

; vetor 8 (console) aponta para c, que conta e segue para o tratador da BIOS
_cons_wrapper:
        ld      r0, (_cnt_console)
        add     r0, 1
        st      r0, (_cnt_console)
        jmp     _bios_trata_console

; chamado pelo despachante da BIOS (convencao da BIOS: r0=num, r1..r4=args).
; Chama int so_syscall(int *quadro, int num, int a1, int a2) na convencao do mcc.
_syscall_trampolim:
        ld      r3, sp
        add     r3, 2
        push    r2
        push    r1
        push    r0
        push    r3
        call    _f_so_syscall
        add     sp, 8
        ret

; interrupcao do relogio: a BIOS chega aqui por JMP com sp = quadro empilhado.
_tick_trampolim:
        ld      r0, sp
        push    r0
        call    _f_so_tick
        add     sp, 2
        rete

_kernel_inicio:
        ld      r1, tabela_syscalls
        ld      r0, _syscall_trampolim
        st      r0, (r1+2)               ; SO_LE
        st      r0, (r1+4)               ; SO_ESCREVE
        st      r0, (r1+6)               ; SO_CRIA_PROC
        st      r0, (r1+8)               ; SO_MATA_PROC
        st      r0, (r1+10)              ; SO_ESPERA_PROC

        ld      r0, _cons_wrapper
        st      r0, (0x40)               ; vetor de interrupcoes, quadro 8: ip

        ld      r0, (_g_ticks_por_fatia)
        ld      r1, r0
        shr     r1, 8
        outb    r1, (0x22)               ; limite do relogio (byte alto)
        outb    r0, (0x23)               ; limite do relogio (byte baixo)
        ld      r0, 5
        outb    r0, (0x30)               ; habilita console (bit 0) e relogio (bit 2)
        ld      r0, _tick_trampolim
        st      r0, (retomada_escalonador)

        ld      r0, pilha_sistema
        add     r0, -32
        push    r0
        call    _f_so_inicia
        add     sp, 2

        ld      sp, pilha_sistema
        add     sp, -32
        rete                             ; entra no init, em modo usuario

        .data
_cnt_console:   .dw 0
