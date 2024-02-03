# Считываем имя
get_name() {
    echo "Введите имя"
    read name
}

hi() {
    get_name
    echo "Введите фамилию:"
}

# Вызываем функцию hi
hi

# Читаем фамилию
read surname

# Выводим приветствие
echo "Здравствуйте, $name $surname"
read -p "Press Enter to exit..."