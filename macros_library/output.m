# .include "../macros_library/common_macros.m" - for separate usage from the main code

.data
output_text: .asciz "2. Enter the name of the output file: "
text_message: .asciz "3. Do you want to display the result on the console? (Enter Y or N): "
first_str: .asciz "    String 1: "
second_str: .asciz "    String 2: "
result_text: .asciz "The result has been saved to the file "
lb: .asciz "\n"

.macro output(%file, %str1, %size1, %str2, %size2)   
   push(s0)
   push(s1)
   push(s2)
   push(s3)
   push(s4)
   
   mv s0 %file              # file descriptor
   mv s1 %str1             # Address of the beginning of string 1
   mv s2 %size2            # Number of elements in string 1
   mv s3 %str2             # Address of the beginning of string 2
   mv s4 %size2            # Number of elements in string 2
   
   mv a0 s0
   mv a1 s1
   mv a2 s2
   write(a0, a1, a2)       # Save string 1 to the output file
   
   mv a0 s0
   la a1 lb
   li a2 1
   write(a0, a1, a2)       # Line break
   
   mv a0 s0
   mv a1 s3
   mv a2 s4
   write(a0, a1, a2)       # Save string 2 to the output file
   
   mv a0 s0
   close(a0)               # Close the file
   
   print_text(text_message)
   input_char
   li t0 'Y'
   beq a0 t0 output_result # If the user entered 'Y', display the strings on the screen
   
   new_line
   print_text(result_text)
   print_text(output_file_name)
  
   j end

output_result:              # Display the strings on the screen
	new_line
	print_text(first_str)
	mv a0 s1
	print_text_reg(a0)
	
	new_line
	print_text(second_str)
	mv a0 s3
	print_text_reg(a0)
	
	new_line
	print_text(result_text)
   	print_text(output_file_name)
   	j end
   	
end:
	pop(s4)
	pop(s3)
	pop(s2)
	pop(s1)
	pop(s0)
.end_macro