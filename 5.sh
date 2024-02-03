# Внутренняя функция
first_function() {
    echo "А вот и первая функция"
}

# Основная функция
second_function() {
    echo "Это вторая функция, она вызывает первую"
    sleep 1
    echo "....."
    echo "Сейчас вызову, 2 секунды плз"
    echo "....."
    sleep 2
    first_function
}

# Вызов основной функции
second_function
read -p "Press Enter to exit..."