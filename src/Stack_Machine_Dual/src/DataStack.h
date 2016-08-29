#ifndef DATASTACK_H_
#define DATASTACK_H_

#include "systemc"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "Defines.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

/**
 * \class DataStack
 * \brief Pilha de dados.
 *
 * Módulo responsável por manter e fazer a interface da pilha de dados
 * com os demais blocos.
 *
 * \author Gerson Miguel Beckenkamp
 * \date 2015/09/05 14:16:20
 * \file DataStack.h
 */
class DataStack : sc_module{
private:
	sc_int<DATA_WIDTH>		RS[STACK_SIZE];	/**<	Pilha de dados contendo 16 posições, o número máximo posições suportados pelos operandos da instrução.*/
	sc_int<DATA_WIDTH>		dsp;			/**<	Endereço do topo da pilha.*/
	sc_int<DATA_WIDTH>		_dsp;			/**<	Endereço auxiliar usado no manejo da pilha.*/

public:

	tlm_utils::simple_target_socket<DataStack> socket;
	tlm_utils::simple_target_socket<DataStack> socketUla;

	SC_CTOR(DataStack);
	virtual void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay);
	~DataStack();
};


#endif /* DATASTACK_H_ */
