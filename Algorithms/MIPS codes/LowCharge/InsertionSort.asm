.data
#A: .word 12,4,2,1,9,7,8,3 	# declaracao do vetor de 8
#A: .word 12, 4, 2, 1, 9, 7, 8, 3, 15, 13, 14, 11, 6, 10, 5, 16 # declaracao do vetor de 8
A: .word 12, 4, 2, 1, 9, 7, 8, 3, 15, 13, 14, 11, 6, 10, 5, 16, 23, 24, 21, 26, 19, 28, 17, 30, 31, 32, 29, 18, 27, 20, 25, 22 # declaracao do vetor de 8

.text
#li 	$t0, 32 # tamanho máximo do vetor de 8
#li 	$t0, 64 # tamanho máximo do vetor de 16
li 	$t0, 128	# tamanho máximo do vetor de 32
li 	$t1, 4
li 	$t2, 4

main:
	lw   	$t3, A($t2)     # Carrega a posição mais ao fim do vetor
	subi 	$t4, $t2, 4     # Calcula o endereço anterior no vetor 
	lw	$t5, A($t4)     	# Carrega o valor no endereço calculado anteriormente
	bgt	$t5, $t3, swap  	# Se o valor mais abaixo no vetor é maior
	j	zero				# Caso contratio

swap:
	sw	$t5, A($t2)		# Swap
	sw	$t3, A($t4)		# Swap
	beqz	$t4, zero	# Se chegou a base do vetor
	move	$t2, $t4	# Senão, carrega o novo endereço endere do vetor
	j	main		

zero:
	addi 	$t1, $t1, 4	# Aumenta o endereço de base para os swaps
	beq	$t0, $t1, exit	# Se chegou ao fim
	move	$t2, $t1	# Carrega o novo enereço para testar
	j	main

exit:
