/**
 * \brief Definição de valores fixos do sistema.
 *
 * \author Gerson Miguel Beckenkamp
 * \date 2015/09/05 14:16:20
 * \file Defines.h
 */

#ifndef SRC_DEFINES_H_
#define SRC_DEFINES_H_

//	Debugs para cada um dos módulos
#define DEBUG_ALU
#define DEBUG_DS
#define DEBUG_FETCH
#define DEBUG_RAM
#define DEBUG_ROM
#define DEBUG_RS
#define DEBUG_DSC

#define PATH					"program.sm"		///< Caminho para o arquivo de programa

#define DEBUG_PRINT_RAM_TOP		32					///< Topo do bloco de RAM que será exibido

#define DEBUG_PRINT_RAM_DOWN	0					///< Base do bloco de RAM que será exibido

#define SIZE 					256					///< Data size

#define DATA_WIDTH        		32					///< Tamanho da palavra do sistema

#define RAM_DEPTH        		128					///< Tamanho da memória RAM

#define ROM_DEPTH        		128					///< Tamanho da memória ROM

#define STACK_SIZE				16					///< Tamanho máximo de pilha suportado

#define DS_SIZE					8					///< Tamanho atual da pilha de dados

#define RS_SIZE					8					///< Tamanho atual da pilha de retorno


#endif /* SRC_DEFINES_H_ */
