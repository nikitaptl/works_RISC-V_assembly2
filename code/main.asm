.include "../macros_library/macros.m"
.include "../macros_library/read_file.asm"
.include "../macros_library/create.asm"

.eqv    NAME_SIZE 256
.eqv 	TEXT_SIZE 512

.data
text: .asciz "input.txt"
input_text: .asciz "Введите имя входного файла: "
incorrect_input_text: .asciz "Вы введи некорректное имя файла или его не существует"
incorrect_file_format_text: .asciz "Некорректный формат файла. Должно быть две строки."
input_file_name:  .space NAME_SIZE	

.text
main:
   print_text(input_text)
   get_str(input_file_name, NAME_SIZE)
   open_file(input_file_name, read_only)
   mv s0 a0 # File descriptor
   
   li t0 -1
   beq s0 t0 incorrect_input
   
   read_file(a0, TEXT_SIZE)
   mv s1 a0 # Начало текста из файла
   mv s2 a1 # Количество символов

   find_lb(a0, a1)
   mv s3 a0 # Сохраняем индекс первого вхождения /n (т.е. индекс, на котором заканчивается 1 строка)
   mv s4 a1 # Сохраняем количество /n (т.е. количество строк, если прибавить 1)
   
   li t0 1 
   bne t0 s4 incorrect_file_format # Если количество строк != 1, то выводим ошибку
   
   mv a0 s1
   mv a1 s3 
   mv a2 s2
   create(a0, a1, a2)

   li a7 4
   ecall
   
   b end

incorrect_input:
	print_text(incorrect_input_text)
	b end

incorrect_file_format:
	print_text(incorrect_file_format_text)
	b end

end:
	li a7 10
	ecall
