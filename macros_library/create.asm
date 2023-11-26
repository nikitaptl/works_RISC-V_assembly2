.macro create(%start, %lb, %num)
	push(s0)
	push(s1)
	push(s2)
	push(s3)
	push(s4)
	push(s5)
	push(s6)
	push(s7)
	push(s8)
	
	mv t0 %start # Итератор на старт
	mv t1 %lb # Индекс перерноса
	mv t2 %num # Конец
	
	mv s0 t0 # Адрес начала 1 строки
	add s1 s0 %lb # Адрес конца 1 строки (начала 2-ой)
	add s2 s0 %num # Адрес конца 2 строки
	
	mv a0 t1
	allocate_reg(a0)
	mv s3 a0 # Адрес начала новой строки 1
	mv s7 s3 # Копия
	li s4 0 # Количество элементов в этой строке 
	
	sub a0 t2 t1
	allocate_reg(a0)
	mv s5 a0 # Адрес начала новой строки 2
	mv s8 s5 # Копия
	li s6 0 # Количество элементов в этой строке
	
	mv t0 s0 # Начало строки 1
	blt t0 s1 loop1
	j find_second
	
	loop1:
		lb t1 (t0) # Элемент строки 1
		
		mv t2 s1 
		check_uniqueness:
			lb t3 (t2) # Элемент строки 2
			beq t1 t3 next # Если они равны, то элемент не уникален и переходим на следующую итерацию
			
			addi t2 t2 1 # Переходим на следующий элемет
			blt t2 s2 check_uniqueness
		j unique
		
		unique:
			sb t1 (s7) # Загружаем жлемент в новую строку 
			addi s7 s7 1
			addi s4 s4 1
			j next
			
		next:
			addi t0 t0 1 # Переходим на следующий элемент 
			blt t0 s1 loop1
			j find_second
			
	
	find_second:	
	mv t0 s1
	blt t0 s2 loop2
	j end
	
	loop2:
		lb t1 (t0) # Элемент строки 2
		
		mv t2 s0 # Начало строки 1
		check_uniqueness2:
			lb t3 (t2) # Элемент строки 1
			beq t1 t3 next2 # Если они равны, то переходим на следующую итерацию
			
			addi t2 t2 1 # Следующий элемент
			blt t2 s1 check_uniqueness2
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
			
	end:
		mv a0 s3
		mv a1 s4
		mv a2 s5
		mv a3 s6
		
		pop(s8)
		pop(s7)
		pop(s6)
		pop(s5)
		pop(s4)
		pop(s3)
		pop(s2)
		pop(s1)
		pop(s0)
	
.end_macro