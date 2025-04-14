#include <stdio.h>
#include "apple.h"

int main() {
    double price = 0.0;
    double kg = 0.0;

    scanf("%lf", &price);

    scanf("%lf", &kg);

    double cost = apple_calculator(price, kg);
    printf("%.2f кг яблок стоят %.2f рублей\n", kg, cost);

    return 0;
}
