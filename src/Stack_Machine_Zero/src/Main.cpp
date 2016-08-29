#include "systemc.h"
#include <iostream>
#include "Alu.h"
#include "DataStack.h"
#include "Fetch.h"
#include "Rom.h"
#include "Ram.h"
#include "ReturnStack.h"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

using namespace std;


/**
 * Módulo de sistema, contendo todos os demais módulos do projeto,
 * bem como suas ligações.
 */
SC_MODULE(Top) {

	Stack			*rs;
	DataStack		*ds;
	Alu				*alu;
	Ram				*ram;

public:

	Fetch			*fet;
	Rom				*rom;
	sc_in_clk 		clk;
	int *PC, *buffersDS, *buffersRS, *cntExec, *ramAcc;

	SC_CTOR(Top) {

		// Inicia variáveis
		cntExec		= (int*) malloc(	sizeof(int));
		ramAcc		= (int*) malloc(	sizeof(int));
		PC			= (int*) malloc(	sizeof(int));
		buffersDS	= (int*) malloc(4 * sizeof(int));
		buffersRS	= (int*) malloc(4 *	sizeof(int));
		*cntExec	= 0;
		*ramAcc		= 0;
		*PC			= 0;

		// Instancia módulos
		rs			= new Stack("Stack");
		ds			= new DataStack("DataStack");
		rom			= new Rom("ROM");
		fet			= new Fetch("Fetch");
		alu			= new Alu("ULA");
		ram			= new Ram("RAM");

		alu->setPC(PC);
		fet->setPC(PC);

		fet->setCnt(cntExec);
		ram->setRamAcc(ramAcc);

		rs->setBuffers(buffersRS);
		alu->setBuffersRS(buffersRS);

		ds->setBuffers(buffersDS);
		alu->setBuffersDS(buffersDS);

		fet->enable(clk);

		// Ligação entre sockets
		fet->socketRom.bind(rom->socket);
		fet->socketULA.bind(alu->socketInFet);

		alu->socketDS.bind(ds->socket);
		alu->socketRAM.bind(ram->socket);
		alu->socketRS.bind(rs->socket);


	}

	~Top() {
		// Libera da memória todos os objetos e variáveis alocados
		delete PC;
		delete buffersDS;
		delete buffersRS;

		free(PC);
		free(buffersDS);
		free(buffersRS);
		free(cntExec);
		free(ramAcc);

	}



};

/**
 * \brief Método main.
 *
 * \author Gerson Miguel Beckenkamp
 * \date 2015/09/05 14:16:20
 * \file Main.cpp
 */
int sc_main(int argc, char* argv[]) {

	Top 				*top	= new Top("top");

	sc_signal<bool> 	enable;
	enable 				= true;

	top->clk(enable);

	sc_start();

	cout<<"\n\n\n Execution cycles: "<<*top->cntExec<<"\n";
	cout<<"\n Total RAM accesses: "<<*top->ramAcc<<"\n";

	delete 	top;

	return 0;
}
