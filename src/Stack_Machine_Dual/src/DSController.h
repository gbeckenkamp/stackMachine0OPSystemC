/*
 * DSController.h
 *
 *  Created on: Aug 20, 2015
 *      Author: nuts
 */

#ifndef DSCONTROLLER_H_
#define DSCONTROLLER_H_

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
 * \class DSController
 * \brief Controlador da pilha de dados.
 *
 * Módulo responsável por determinar quais operandos devem ser carregados
 * da pilha de dados e obter estes operandos da mesma.
 *
 * \author Gerson Miguel Beckenkamp
 * \date 2015/09/05 14:16:20
 * \file DSController.h
 */
class DSController : sc_module{
private:

	sc_int<DATA_WIDTH>		instruction;	/**<	Instrução atual.*/
	int						load;			/**<	Flag utilizada para detectar se há necessidade de buscar operandos.*/
	bool					search;			/**<	Flag utilizada para controlar a leitura de operandos.*/

	void thread_process();
	void make_end();
	void make_Transaction( int addr,int buffer_position);


public:

	tlm_utils::simple_target_socket<DSController>		socketIn;
	tlm_utils::simple_initiator_socket<DSController>	socketOutDS;
	tlm_utils::simple_initiator_socket<DSController>	socketOutUla;

	SC_CTOR(DSController);
	virtual void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay);
	~DSController();

};



#endif /* DSCONTROLLER_H_ */
