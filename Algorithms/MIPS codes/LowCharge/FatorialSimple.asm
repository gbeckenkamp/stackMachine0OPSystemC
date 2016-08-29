.text

main:
	#li $a0, 7	# Fatorial de 7
	#li $a2, 7	# Primeiro valor a ser multiplicado
	#li $a0, 14	# Fatorial de 7
	#li $a2, 14	# Primeiro valor a ser multiplicado
	li $a0, 28	# Fatorial de 7
	li $a2, 28	# Primeiro valor a ser multiplicado
	j next

next:
	subi $a1, $a0 , 1	# Subtrai o fatorial atual
	beqz $a1, exit		# Se igual a zero termina
	mul  $a2, $a2, $a1	# Caso contrario multiplica
	move $a0, $a1		# Substitui com o novo fatorial 
	j next

exit:
