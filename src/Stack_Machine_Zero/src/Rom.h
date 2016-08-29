/*
 * ROM.h
 *
 *  Created on: Aug 13, 2015
 *      Author: nuts
 */

#ifndef ROM_H_
#define ROM_H_

#include "systemc"
#include "Defines.h"
#include "FileHandler.cpp"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;


/**
 * \class Rom
 * \brief Read-Only Memory.
 *
 * Módulo responsável por manter e fazer a interface entre a memória ROM com os demais módulos.
 *
 * \author Gerson Miguel Beckenkamp
 * \date 2015/09/05 14:16:20
 * \file Rom.h
 */
class Rom : sc_module{
private:

	sc_int<DATA_WIDTH> 	mem[ROM_DEPTH];		/**<	Memória ROM.*/
	FileHandler			*fh;				/**<	Gerenciador de arquivos.*/

	virtual void b_transport(tlm::tlm_generic_payload& transaction, sc_time& delay);

public:

	tlm_utils::simple_target_socket<Rom> socket;

	SC_CTOR(Rom);
	~Rom();
	void setPath(char* path);

};

#endif /* ROM_H_ */
