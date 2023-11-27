.macro find_lb(%start, %num)
	li t0 '\n'                # Load the ASCII value of '\n' into t0
	li t1 0                   # Index of the first occurrence of \n
	li t2 0                   # Counter
	mv t3 %start              # Address of the beginning of the string
	mv t4 %num                # For comparison
	li t5 0                   # Number of \n in the file

	loop:
		lb a0 (t3)             # Load the byte at the address t3 into a0
		beq a0 t0 sym_lb       # If the character is \n, jump to the corresponding branch

		checking:              # Loop condition check
		addi t2 t2 1
		addi t3 t3 1
		blt t2 t4 loop
		j end_loop

		sym_lb:
			addi t5 t5 1        # Increment the count of \n in the string by 1
			li t6 1

			beq t5 t6 first_entry
			j checking

			first_entry:         # If this is the first occurrence of \n in the string, store the index in t1
				mv t1 t2
				j checking

	end_loop:
		mv a0 t1                # Return value - index of the first occurrence of \n in the string
		mv a1 t5                # Return value - number of \n in the string
		j end
	end:
.end_macro 