.include "../macros_library/common_macros.m"
.include "../macros_library/read_file.m"
.include "../macros_library/find_lb.m"
.include "../macros_library/create_result.m"
.include "../macros_library/output.m"

.eqv    NAME_SIZE 256
.eqv 	TEXT_SIZE 512

.data
text: .asciz "input.txt"
input_text: .asciz "1. Enter the name of the input file: "
incorrect_input_text: .asciz "You entered an incorrect file name or it does not exist"
incorrect_file_format_text: .asciz "Incorrect file format. The file should contain two lines"
input_file_name:  .space NAME_SIZE	
output_file_name: .space NAME_SIZE

.text
main:
   print_text(input_text)
   get_str(input_file_name, NAME_SIZE) # Input the name of the input file via the console
   open_file(input_file_name, read_only) # Open the file
   mv s0 a0 # File descriptor
   
   li t0 -1 
   beq s0 t0 incorrect_input # If the file descriptor = -1, then the file did not open
   
   read_file(a0, TEXT_SIZE) # Read the file
   mv s1 a0 # Start of the text from the file
   mv s2 a1 # Number of characters
   close(a0) # Close the file
 
   find_lb(a0, a1) # Call the subroutine that finds the index of the first occurrence of /n and their count
   mv s3 a0 # Save the index of the first occurrence of /n (i.e., the index where the 1st line ends)
   mv s4 a1 # Save the count of /n (i.e., the number of lines, if you add 1)
   
   li t0 1 
   bne t0 s4 incorrect_file_format # If the number of lines != 1, then output an error
   
   mv a0 s1
   mv a1 s3 
   mv a2 s2
   create_result(a0, a1, a2) # Call the subroutine that creates two lines according to the task
   mv s1 a0 # Save the address of the new line 1
   mv s2 a1 # Save the number of elements in the new line 1
   mv s3 a2 # Save the address of the new line 2
   mv s4 a3 # Save the address of the new line 1

   print_text(output_text)
   get_str(output_file_name, NAME_SIZE) # Input the output file name
   open_file(output_file_name, write_only) # Open the file
   mv s0 a0 # File descriptor
   
   mv a1 s1
   mv a2 s2
   mv a3 s3
   mv a4 s4
   output(a0, a1, a2, a3, a4) # Call subroutines to output all the obtained information to the screen
   
   j end
 		
incorrect_input: # Branch for entering an incorrect file name
	print_text(incorrect_input_text)
	j end

incorrect_file_format: # Branch for an incorrect file format (<=> the number of lines is not equal to 2)
	print_text(incorrect_file_format_text)
	j end

end: 
	li a7 10 # Exit the program
	ecall
