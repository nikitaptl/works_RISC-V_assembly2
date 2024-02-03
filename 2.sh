num=1

# Входим цикл, с условием num < 1 и прибавкой к num++
while [ $num -le 10 ]; do
    echo "Число = $num"
    ((num++))
done
read -p "Press Enter to exit..."	