#
# Inputs two integers and prints their sum.
#
.data	
checker:
	.word 0x80000000	

    .text
main:
    # x: t0 = readInt()
    li a7, 5
    ecall
        
    mv t0, a0

    # y: t1 = readInt()
    li a7, 5
    ecall
    
    mv t1, a0

    # z: t2 = t0 + t1
    add t2, t0, t1

    # printInt(t2)   
    mv a0, t2
    li a7, 1  
    ecall
    
    # 0x8000000 - 16-ричная система счисления (8 = 1000, 0 = 0000)
    
    lw t5, checker
    and s0, t0, t5 #s0 = t0 (input 1) & 0x80000000
    and s1, t1, t5 #s1 = t1 (input 2) & 0x80000000 #тут мы оставляем один бит
    xor s2, s1, s0 # s2 = (s0 ^ s1) # и тут проверяем что если у обеих цифр знак одинаковый 
    
    and s3, t2, t5 #s3 = t2 (our sum t0+t1) & 0x80000000 
    
    xor s4, s3, s1 # тут делаем то же самое и в s4 будет 0 если знак суммы и любого слагаемого одинаковые если 1 то знаки слагаемых разные 
    
    srli s5, s5, 31 #srli - сдвиг бита вправо чтобы у нас была либо 1 либо 0
    
    li a0, '\n' # загрузка константы на регистр а0
    li a7, 11   # 11 - print char
    ecall
    
    mv a0, s5 # системные вызовы не видят регистры кроме a0-a7 поэтому мы переносим 
    li a7, 1 # 1 - print int
    ecall
    
