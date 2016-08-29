/*
 * DataStack.h
 *
 *  Created on: Aug 13, 2015
 *      Author: nuts
 */

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
class DataStack : sc_module {
private:

	sc_int<DATA_WIDTH>		DS[STACK_SIZE];		/**<	Pilha de dados.*/
	sc_int<DATA_WIDTH>		sp;					/**<	Endereço de topo da pilha.*/
	int						*buffersDS;			/**<	Array de buffers do topo da pilha.*/

public:

	tlm_utils::simple_target_socket<DataStack> socket;

	void setBuffers(int *buf);
	void setDsp(int *dsp);
	void setDss(int *dss);
	virtual void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay);
	void updateBuffers();

	SC_CTOR(DataStack);
	~DataStack();

};

#endif /* DATASTACK_H_ */
