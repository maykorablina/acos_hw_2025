#!/bin/sh

sum() {
    total=0
    for val in "$@"; do
        num=$(expr "$val" + 0 2>/dev/null)
        if [ $? -ne 0 ]; then
            echo 0
            return
        fi
        total=$(expr "$total" + "$num" 2>/dev/null)
    done
    echo "$total"
}

# читаем числа с двух строк
read -r line1
read -r line2

# считаем их суммы
sum1=$(sum $line1)
sum2=$(sum $line2)

# суммы равны или не равны
if [ "$sum1" -eq "$sum2" ]; then
    echo "Sums are equal"
else
    echo "Sums are NOT equal"
fi
