.include "../macros_library/common_macros.m"

.data:
error_read_text: .asciz "Read error!"

.macro read_file(%file_descriptor, %text_size)
	   push(s0) # Save the s-registers used on the stack
	   push(s1)
	   push(s2)
	   push(s3)
	   
	   mv s3 %file_descriptor
	   
	   allocate(%text_size) # Allocate memory for the specified buffer size
	   mv s0 a0 # Fix this register to store the address of the beginning of the text from the file in memory
	   mv s1 a0 # Mutable register to store the end
	   li s2 0 # Counter for the number of characters
	   
	   li t0 TEXT_SIZE
	   li t2 -1
	   
	   reading_loop: # Reading similar to the file load_text.asm from seminars
	   	read(s3, s1, %text_size)
	   	beq a0 t2 error_read # If = -1, then a read error occurred, exit the program
	   	
	   	mv t1 a0
	   	add s2 s2 t1
	   	
	   	bne t0 t1 reading_end
	   	
	   	allocate(TEXT_SIZE)
	   	add s1 s1 s2
	   	b reading_loop
	
	error_read:
		print_text(error_read_text)
		li a7 10
		ecall
		
	reading_end:
		mv a0 s0 # Return value - address of the beginning of the string 
		mv a1 s2 # Return value - number of elements in the string
		pop(s3) # Retrieve saved s-registers from the stack
		pop(s2)
		pop(s1)
		pop(s0)
		j end
	end:
.end_macro