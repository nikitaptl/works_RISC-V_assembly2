.include "..\macros_library\macros.asm"
.eqv SIZE 256

.data
str1: .space SIZE
str2: .space SIZE
text_start: .asciz "Hello! Enter the original string (not more than 256 characters): "
text_end: .asciz "Your string has been successfully copied: "

.text:
main:	
	print_str(text_start)
	str_get(str1, SIZE)
	
	strcpy(str1, str2)
	print_str(text_end)
	print_str(str2)
	
	exit