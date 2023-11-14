.macro print_str (%x)
   li a7, 4
   la a0, %x
   ecall
.end_macro

.macro newline
   print_char('\n')
.end_macro

.macro pop(%x)
	lw	%x, (sp)
	addi	sp, sp, 4
.end_macro

.macro push(%x)
	addi	sp, sp, -4
	sw	%x, (sp)
.end_macro

.macro exit
    li a7, 10
    ecall
.end_macro


.macro str_get(%strbuf, %size)
    la      a0 %strbuf
    li      a1 %size
    li      a7 8
    ecall
    push(s0)
    push(s1)
    push(s2)
    li	s0 '\n'
    la	s1	%strbuf
next:
    lb	s2  (s1)
    beq s0	s2	replace
    addi s1 s1 1
    b	next
replace:
    sb	zero (s1)
    pop(s2)
    pop(s1)
    pop(s0)
.end_macro

.macro strcpy(%bufer1, %bufer2)
la t1 %bufer1
la t2 %bufer2

copying:
	lb t3 (t1)
	beqz t3 end
	sb t3 (t2)
	addi t1 t1 1
	addi t2 t2 1
	j copying
end_copying:
	j end
end:
.end_macro
