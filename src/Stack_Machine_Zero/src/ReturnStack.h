/*
 * Stack.h
 *
 *  Created on: Aug 13, 2015
 *      Author: nuts
 */

#ifndef RETURNSTACK_H_
#define RETURNSTACK_H_


#include "systemc"
#include "Defines.h"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

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
 * \file ReturnStack.h
 */
class Stack : sc_module {
private:

	sc_int<DATA_WIDTH>		RS[RS_SIZE];
	sc_int<DATA_WIDTH>		sp;
	int				*buffersRS;

	virtual void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay);
	void updateBuffers();

public:

	tlm_utils::simple_target_socket<Stack> socket;

	SC_CTOR(Stack);
	void setRsp(int *pc);
	void setRss(int *pc);
	void setBuffers( int *buf);
	~Stack();

};

#endif /* RETURNSTACK_H_ */
