#ifndef FETCH_H_
#define FETCH_H_

#include "systemc"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "Defines.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

/**
 * \class Fetch
 * \brief Interface da ROM com o resto do sistema.
 *
 * Módulo responsável por controlar o módulo ROM, carregando as instruções
 * e repasando-as para os módulos Alu e DSController.
 *
 * \author Gerson Miguel Beckenkamp
 * \date 2015/09/05 14:16:20
 * \file Fetch.h
 */

class Fetch : sc_module{
private:

	sc_int<DATA_WIDTH>	instruction;	/**<	Instrução atual.*/
	int					*PC;			/**<	Contador de programa.*/
	bool				end;			/**<	Flag utilizada para detectar o fim de execução.*/
	int					*cntExec;		/**<	Contador de ciclos de execução.*/

	void transaction_Read();
	void transaction_Write();
	void thread_process();

public:

	sc_in_clk									enable;
	tlm_utils::simple_initiator_socket<Fetch>	socketRom;
	tlm_utils::simple_initiator_socket<Fetch>	socketDs;
	tlm_utils::simple_initiator_socket<Fetch>	socketULA;

	SC_CTOR(Fetch);
	void setPC(int *pc);
	void setCnt(int *cnt);
	~Fetch();

};



#endif /* FETCH_H_ */
