main:
	#li $a0, 7				# Número na série
	#li $a0, 14				# Número na série
	li $a0, 28				# Número na série
	li $a1, 0				# Contador de iterações
	j fib

fib:
	beq $a1, $a0, end		# Se foram percorridos todos os numeros
	bgtz $a1, calcule		# Se maior que zero, calcula o próximo valor na série
	j start

calcule:

	add $t2, $t0, $t1		# Calcula o novo número na série
	move $t0, $t1			# Transfere a série anterior
	move $t1, $t2			# Transfere a série anterior
	addi $a1, $a1, 1		# Conta mais uma iteração
	j fib

start:
	li $t1, 1				# Carrega 1
	addi $a1, $a1, 1		# Add com o número da série anterior
	j fib

end:

