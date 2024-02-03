# Получаем текущий час
echo "Введите час, от 0 до 24"
read current_hour

# Проверяем, находится ли час в отрезке от 18 до 24
if [ $current_hour -ge 18 ] && [ $current_hour -lt 24 ]; then
    echo "Уже поздно, на улице темно."
else
    echo "Еще светло на улице."
fi
read -p "Press Enter to exit..."
