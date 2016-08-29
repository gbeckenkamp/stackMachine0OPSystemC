/*
 * Alu.h
 *
 *  Created on: Aug 14, 2015
 *      Author: nuts
 */

#ifndef ALU_H_
#define ALU_H_


#include <stdlib.h>
#include <cstdlib>
#include "systemc"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "Defines.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

/**
 * \class Alu
 * \brief Unidade Lógico Aritmética.
 *
 * Módulo responsável pela computação dos valores carregados das
 * pilhas e por alocar o resultado na pilha de destino ou em memória.
 *
 * \author Gerson Miguel Beckenkamp
 * \date 2015/09/05 14:16:20
 * \file Alu.h
 */
class Alu: sc_module {
private:

	sc_int<DATA_WIDTH>	instruction;		/**<	Instrução atual.*/
	int					*PC;				/**<	Contador de programa.*/
	int					*buffersDS;			/**<	Buffer contendo as 4 posições mais ao topo da pilha de dados.*/
	int					*buffersRS;			/**<	Buffer contendo as 4 posições mais ao topo da pilha de retorno.*/
	int					*DSP;				/**<	Ponteiro para o endereço da base das pilha de dados na RAM.*/
	int					*RSP;				/**<	Ponteiro para o endereço da base das pilha de retorno na RAM.*/
	int					*DSS;				/**<	Ponteiro para a última posição atualizada do topo da pilhas de dados.*/
	int					*RSS;				/**<	Ponteiro para a última posição atualizada do topo da retorno na memória.*/

	void compute();
	void writeDS(int adr, int op);
	void writeRS(int adr, int op);
	void writeRS(int adr, tlm::tlm_command cmd);
	void writeDS(int adr, tlm::tlm_command cmd);
	void writeRAM(int addr, int op);
	int  readRAM(int addr);
	int  pickSource(int op);
	void storeSource(int op, int data);

public:

	tlm_utils::simple_target_socket<Alu>		socketInFet;
	tlm_utils::simple_initiator_socket<Alu>		socketRS;
	tlm_utils::simple_initiator_socket<Alu>		socketRAM;
	tlm_utils::simple_initiator_socket<Alu>		socketDS;

	void setBuffersDS(int buf[4]);
	void setBuffersRS(int buf[4]);
	void setPC(int *pc);
	void setDSP(int *dsp);
	void setRSP(int *rsp);
	void setSizes(int *dss, int *rss);
	virtual void b_transport_Fet(tlm::tlm_generic_payload& trans,
			sc_time& delay);
	SC_CTOR(Alu);
	~Alu();

};

#endif /* ALU_H_ */
