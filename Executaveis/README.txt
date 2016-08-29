Para executar um programa, crie um arquivo binário com o assembler referente ao código (0op ou 2op). Crie este binário ou renomeie-o para program.sm e coloque-o no mesmo diretório da máquina desejada. Execute a máquina desejada com o comando:

./Stack_Machine_Zero

ou 

./Stack_Machine_Dual

O arquivo binário do programa a ser executado não precisa ser passado como parâmetro. Caso queira trocar o caminho do binário, basta trocar a variável PATH do arquivo defines.h da máquina desejada. Para recompilar a máquina, adicione o projeto ao eclipse CDT e compile-o por lá, ou simplesmente vá até a pasta Debug e execute o makefile.