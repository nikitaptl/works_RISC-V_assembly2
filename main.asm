li a7 5
ecall

mv t0 a0

li a1 0xffff0011
mv a0 t0
sb a0 (a1)
li a1 0xffff0010
	
li a1 0xffff0010
mv a0 t0
sb a0 (a1)
li a1 0xffff0011

li a7 10
ecall
