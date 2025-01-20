# Write a program that inputs two integer values x and y,
# swaps them with the XOR operation, and outputs them back.

.data #все константы

main:
	li a7, 5
	ecall
	mv t0, a0
	
	li a7, 5
	ecall
	mv t1, a0
	
	xor t0, t0, t1
	xor t1, t0, t1
	xor t0, t0, t1
	
	mv a0, t0
	li a7, 1
	ecall
	
	mv a0, t1
	li a7, 1
	ecall
