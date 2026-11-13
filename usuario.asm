; ===== código gerado por mcc =====
.text
_f_escreve:
	push bp
	ld bp, sp
	ld r0, 0
	push r0
	ld r0, (bp+4)
	push r0
	ld r0, 2
	push r0
	call _f_so_chama
	add sp, 6
	ld sp, bp
	pop bp
	ret
_f_le:
	push bp
	ld bp, sp
	ld r0, 0
	push r0
	ld r0, 0
	push r0
	ld r0, 1
	push r0
	call _f_so_chama
	add sp, 6
	ld sp, bp
	pop bp
	ret
	ld sp, bp
	pop bp
	ret
_f_cria_proc:
	push bp
	ld bp, sp
	ld r0, 0
	push r0
	ld r0, (bp+4)
	push r0
	ld r0, 3
	push r0
	call _f_so_chama
	add sp, 6
	ld sp, bp
	pop bp
	ret
	ld sp, bp
	pop bp
	ret
_f_mata_proc:
	push bp
	ld bp, sp
	ld r0, 0
	push r0
	ld r0, (bp+4)
	push r0
	ld r0, 4
	push r0
	call _f_so_chama
	add sp, 6
	ld sp, bp
	pop bp
	ret
	ld sp, bp
	pop bp
	ret
_f_espera_proc:
	push bp
	ld bp, sp
	ld r0, 0
	push r0
	ld r0, (bp+4)
	push r0
	ld r0, 5
	push r0
	call _f_so_chama
	add sp, 6
	ld sp, bp
	pop bp
	ret
	ld sp, bp
	pop bp
	ret
_f_fim_processo:
	push bp
	ld bp, sp
	ld r0, 0
	push r0
	call _f_mata_proc
	add sp, 2
	ld sp, bp
	pop bp
	ret
_f_calcula:
	push bp
	ld bp, sp
	sub sp, 4
	ld r0, 0
	st r0, (bp+-4)
	ld r0, 0
	st r0, (bp+-2)
_L7775d45b_5:
	ld r0, (bp+-2)
	push r0
	ld r0, (bp+4)
	ld r1, r0
	pop r0
	cmp r0, r1
	jmpc lt, _L7775d45b_8
	ld r0, 0
	jmp _L7775d45b_9
_L7775d45b_8:
	ld r0, 1
_L7775d45b_9:
	cmp r0, 0
	jmpc eq, _L7775d45b_7
	ld r0, (bp+-4)
	push r0
	ld r0, (bp+-2)
	ld r1, r0
	pop r0
	add r0, r1
	st r0, (bp+-4)
_L7775d45b_6:
	ld r0, (bp+-2)
	push r0
	add r0, 1
	st r0, (bp+-2)
	pop r0
	jmp _L7775d45b_5
_L7775d45b_7:
	ld sp, bp
	pop bp
	ret
_f_trabalha:
	push bp
	ld bp, sp
	sub sp, 2
	ld r0, 0
	st r0, (bp+-2)
_L7775d45b_15:
	ld r0, (bp+-2)
	push r0
	ld r0, (bp+6)
	ld r1, r0
	pop r0
	cmp r0, r1
	jmpc lt, _L7775d45b_18
	ld r0, 0
	jmp _L7775d45b_19
_L7775d45b_18:
	ld r0, 1
_L7775d45b_19:
	cmp r0, 0
	jmpc eq, _L7775d45b_17
	ld r0, 400
	push r0
	call _f_calcula
	add sp, 2
	ld r0, (bp+4)
	push r0
	call _f_escreve
	add sp, 2
_L7775d45b_16:
	ld r0, (bp+-2)
	push r0
	add r0, 1
	st r0, (bp+-2)
	pop r0
	jmp _L7775d45b_15
_L7775d45b_17:
	ld sp, bp
	pop bp
	ret
_f_proc_a:
	push bp
	ld bp, sp
	ld r0, 6
	push r0
	ld r0, 65
	push r0
	call _f_trabalha
	add sp, 4
	ld sp, bp
	pop bp
	ret
_f_proc_b:
	push bp
	ld bp, sp
	ld r0, 6
	push r0
	ld r0, 66
	push r0
	call _f_trabalha
	add sp, 4
	ld sp, bp
	pop bp
	ret
_f_proc_c:
	push bp
	ld bp, sp
	ld r0, 2
	push r0
	ld r0, 67
	push r0
	call _f_trabalha
	add sp, 4
	ld sp, bp
	pop bp
	ret
_f_proc_filho:
	push bp
	ld bp, sp
	ld r0, 3
	push r0
	ld r0, 102
	push r0
	call _f_trabalha
	add sp, 4
	ld sp, bp
	pop bp
	ret
_f_proc_pai:
	push bp
	ld bp, sp
	sub sp, 2
	ld r0, _f_proc_filho
	push r0
	call _f_cria_proc
	add sp, 2
	st r0, (bp+-2)
	ld r0, 1
	push r0
	ld r0, 80
	push r0
	call _f_trabalha
	add sp, 4
	ld r0, (bp+-2)
	push r0
	call _f_espera_proc
	add sp, 2
	ld r0, 112
	push r0
	call _f_escreve
	add sp, 2
	ld sp, bp
	pop bp
	ret
_f_proc_init:
	push bp
	ld bp, sp
	sub sp, 8
	ld r0, _f_proc_a
	push r0
	call _f_cria_proc
	add sp, 2
	st r0, (bp+-2)
	ld r0, _f_proc_b
	push r0
	call _f_cria_proc
	add sp, 2
	st r0, (bp+-4)
	ld r0, _f_proc_c
	push r0
	call _f_cria_proc
	add sp, 2
	st r0, (bp+-6)
	ld r0, _f_proc_pai
	push r0
	call _f_cria_proc
	add sp, 2
	st r0, (bp+-8)
	ld r0, (bp+-2)
	push r0
	call _f_espera_proc
	add sp, 2
	ld r0, (bp+-4)
	push r0
	call _f_espera_proc
	add sp, 2
	ld r0, (bp+-6)
	push r0
	call _f_espera_proc
	add sp, 2
	ld r0, (bp+-8)
	push r0
	call _f_espera_proc
	add sp, 2
	ld r0, 10
	push r0
	call _f_escreve
	add sp, 2
	ld sp, bp
	pop bp
	ret

.data

; ----- literais de string -----
