#!/bin/bash

process_numbers() {
    num=1

    while [ $num -le 10 ]; do
        remainder=$((num % 2))
        if [ $remainder -eq 0 ]; then
            echo "$num - чётное число!"
        else
            echo "$num - нечётное число!"
        fi
        ((num++))
    done
}

# Вызываем функцию для обработки чисел
process_numbers

# Ожидаем ввода перед выходом
read -p "Press Enter to exit..."