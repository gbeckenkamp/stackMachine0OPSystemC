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

	sc_int<DATA_WIDTH>	instruction;	/**<	Instrução atual.*/
	sc_int<DATA_WIDTH>	buffers[2];		/**<	Buffers utilizados para carregar os operandos da pilha de dados.*/
	int					*PC;			/**<	Contador de programa.*/
	bool				fet_ready;		/**<	Flag referente a comunicação com o módulo Fetch.*/
	bool				buff_ready;		/**<	Flag referente a comunicação com o módulo DSController.*/

	void compute();
	void readRAM(int addr);
	void writeRAM(int addr, int op);
	void writeRS(int op);
	int  readRS(int adr);
	void writeDS(int op);

public:

	tlm_utils::simple_target_socket<Alu>	socketInFet;
	tlm_utils::simple_target_socket<Alu>	socketInDS;
	tlm_utils::simple_initiator_socket<Alu> socketRS;
	tlm_utils::simple_initiator_socket<Alu> socketRAM;
	tlm_utils::simple_initiator_socket<Alu> socketDS;

	virtual void b_transport_Fet(tlm::tlm_generic_payload& trans,
			sc_time& delay);
	virtual void b_transport_DS(tlm::tlm_generic_payload& trans,
			sc_time& delay);
	SC_CTOR(Alu);
	void setPC(int *pc);
	~Alu();

};

#endif /* ALU_H_ */
