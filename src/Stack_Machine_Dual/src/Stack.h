#ifndef STACK_H_
#define STACK_H_

#include "systemc"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "Defines.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

/**
 * \class Stack
 * \brief Pilha de retorno.
 *
 * Módulo responsável por manter e fazer a interface da pilha de retorno
 * com os demais blocos.
 *
 * \author Gerson Miguel Beckenkamp
 * \date 2015/09/05 14:16:20
 * \file Stack.h
 */
class Stack : sc_module{
private:

	sc_int<DATA_WIDTH>		RS[STACK_SIZE];		/**<	Pilha de retorno.*/
	sc_int<DATA_WIDTH>		rsp;				/**<	Endereço do topo da pilha.*/
	sc_int<DATA_WIDTH>		_rsp;				/**<	Endereço auxiliar usado no manejo da pilha.*/

public:

	SC_CTOR(Stack);
	tlm_utils::simple_target_socket<Stack> socket;
	virtual void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay);
	~Stack();

};



#endif /* STACK_H_ */
