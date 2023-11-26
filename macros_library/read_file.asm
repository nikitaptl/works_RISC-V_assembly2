.include "macros.m"

.data:
error_read_text: .asciz "Ошибка чтения!"

.macro read_file(%file_descriptor, %text_size)
	   push(s0)
	   push(s1)
	   push(s2)
	   push(s3)
	   
	   mv s3 %file_descriptor
	   
	   allocate(%text_size)
	   mv s0 a0 # Фиксируем этот регистр для хранения адреса начала текста из файла в памяти
	   mv s1 a0 # Изменяемый регистр для хранения конца
	   li s2 0 # Счётчик количества символов
	   
	   li t0 TEXT_SIZE
	   li t2 -1
	   
	   reading_loop:
	   	read(s3, s1, %text_size)
	   	beq a0 t2 error_read
	   	
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
		mv a0 s0
		mv a1 s2
		pop(s3)
		pop(s2)
		pop(s1)
		pop(s0)
		j end
	end:
.end_macro
