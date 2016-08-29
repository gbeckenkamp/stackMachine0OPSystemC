#ifndef RAM_H_
#define RAM_H_

#include "systemc"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "Defines.h"

using namespace std;
using namespace sc_core;
using namespace sc_dt;

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
class Ram : sc_module {
private:

	sc_uint<DATA_WIDTH> mem[RAM_DEPTH];		/**<	Memória.*/
	int					*cntRamAcc;			/**<	Contador de acessos a RAM.*/

public:

	tlm_utils::simple_target_socket<Ram> socket;SC_CTOR(Ram);
	virtual void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay);
	void setCnt(int *cnt);
	~Ram();
};

#endif /* RAM_H_ */
