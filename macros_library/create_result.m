.macro create_result(%start, %lb, %num)
	push(s0) # Save all registers used in the subroutine on the stack
	push(s1)
	push(s2)
	push(s3)
	push(s4)
	push(s5)
	push(s6)
	push(s7)
	push(s8)
	
	mv t0 %start # Address of the beginning of line 1
	mv t1 %lb # Index of the line break \n
	mv t2 %num # Index of the end of line 2
	
	mv s0 t0 # Address of the beginning of line 1
	add s1 s0 %lb # Address of the end of line 1 (beginning of line 2)
	add s2 s0 %num # Address of the end of line 2
	
	mv a0 t1
	allocate_reg(a0) # Allocate memory for the new line 1 (number of characters <= length of line 1)
	mv s3 a0 # Address of the beginning of the new line 1
	mv s7 s3 # Copy
	li s4 0 # Number of elements in this line 
	
	sub a0 t2 t1 
	allocate_reg(a0) # Allocate memory for the new line 2 (number of characters <= length of line 2)
	mv s5 a0 # Address of the beginning of the new line 2
	mv s8 s5 # Copy
	li s6 0 # Number of elements in this line
	
	mv t0 s0 # Start of line 1
	blt t0 s1 loop1
	j find_second
	
	loop1: # Loop to create the new line 1
		lb t1 (t0) # Element of line 1
		
		mv t2 s1 
		check_uniqueness: # Check if the element occurs in line 2
			lb t3 (t2) # Element of line 2
			beq t1 t3 next # If they are equal, the element is not unique, and we go to the next iteration
			
			addi t2 t2 1 # Move to the next element
			blt t2 s2 check_uniqueness
		j check_if_first
		
		check_if_first: # Check if the element is unique among already obtained unique elements
			mv a4 s3
			blt a4 s7 loop_checking
			
			j unique
		
			loop_checking:
				lb t3 (a4)
				beq t1 t3 next # Similar actions as above
				
				addi a4 a4 1
				blt a4 s7 loop_checking
				
			j unique
		
		unique:	# If the element is unique, then we enter this branch
			sb t1 (s7) # Load the element into the new line 
			addi s7 s7 1
			addi s4 s4 1
			j next
			
		next:
			addi t0 t0 1 # Move to the next element 
			blt t0 s1 loop1
			j find_second
			
	
	find_second: # Similarly, create the new line 2 in this branch
		
	mv t0 s1
	addi t0 t0 1
	blt t0 s2 loop2
	j end
	
	loop2: # Similarly
		lb t1 (t0)
		
		mv t2 s0 
		check_uniqueness2:
			lb t3 (t2) 
			beq t1 t3 next2 
			
			addi t2 t2 1
			blt t2 s1 check_uniqueness2

		j check_if_first2
		
		check_if_first2:
			mv a4 s5
			blt a4 s8 loop_checking2
			
			j unique2
		
			loop_checking2:
				lb t3 (a4)
				beq t1 t3 next2
				
				addi a4 a4 1
				blt a4 s8 loop_checking2
				
			j unique2
		
		unique2:
			sb t1 (s8)
			addi s8 s8 1
			addi s6 s6 1
			j next2
			
		next2:
			addi t0 t0 1
			blt t0 s2 loop2
			j end
			
	end: # End of the subroutine
		mv a0 s3 # Return value - address of the beginning of the new line 1
		mv a1 s4 # Return value - number of elements in the new line 1
		mv a2 s5 # Return value - address of the beginning of the new line 2
		mv a3 s6 # Return value - number of elements in the new line 2
		
		pop(s8) # Retrieve saved s-registers from the stack
		pop(s7)
		pop(s6)
		pop(s5)
		pop(s4)
		pop(s3)
		pop(s2)
		pop(s1)
		pop(s0)
	
.end_macro