.data
#A: .word 12,4,2,1,9,7,8,3 	# teste com vetor de 8
#A: .word 12, 4, 2, 1, 9, 7, 8, 3, 15, 13, 14, 11, 6, 10, 5, 16 # teste com vetor de 16
A: .word 12, 4, 2, 1, 9, 7, 8, 3, 15, 13, 14, 11, 6, 10, 5, 16, 23, 24, 21, 26, 19, 28, 17, 30, 31, 32, 29, 18, 27, 20, 25, 22 # teste com vetor de 32

.text
li $a0, 4	
#li $t0, 28 # endereço máximo para teste com vetor de 8
#li $t0, 60 # endereço máximo para teste com vetor de 16
li $t0, 124 # endereço máximo para teste com vetor de 32
li $t1, 0	# endereço inicial
Main:
	addi $t2, $t1, 4	# pega o próximo endereço do vetor
	lw $s1, A($t1)		# carrega o endereço mais baixo
	lw $s0, A($t2)		# carrega o endereço mais alto
	bgt $s1, $s0, swap	# caso a posição mais abaixo do vetor seja menor, realiza um swap
	j test1				# testa a cintinuidade
	
swap:					# inverte as posições do vetor
	sw $s0, A($t1)		
	sw $s1, A($t2)
	j test1
	
test1:					# testa se chegou ao fim do vetor
	beq $t2, $t0, test2	# caso chegou ao final do vetor, testa o fim
	move $t1, $t2		# senão, avança o endereço atual
	j Main
	
test2:					# testa o fim da execução
	subi $t0, $t0, 4	# diminui o tamanho do vetor a ser analisado
	beq $t0, $a0, exit	# caso chegue na primeira posição, termina
	li $t1, 0			# caso contrário, recomeça o loop
	j Main
	
exit: