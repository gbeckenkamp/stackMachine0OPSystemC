/*
 * Ram.h
 *
 *  Created on: Aug 16, 2015
 *      Author: nuts
 */

#ifndef RAM_H_
#define RAM_H_

#include "systemc"
#include "Defines.h"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

/**
 * \class Ram
 * \brief Random Access Memory.
 *
 * Módulo responsável por manter e fazer a interface entre a memória RAM com os demais módulos.
 *
 * \author Gerson Miguel Beckenkamp
 * \date 2015/09/05 14:16:20
 * \file Ram.h
 */
class Ram : sc_module{
private:

	sc_uint<DATA_WIDTH> mem[RAM_DEPTH];			/**<	Memória.*/
	int					*cntRamAcc;				/**<	Contador de acessos a RAM.*/

	virtual void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay);

public:

	tlm_utils::simple_target_socket<Ram> socket;
	void setRamAcc(int *acc);
	SC_CTOR(Ram) ;
	~Ram();

};

#endif /* RAM_H_ */
