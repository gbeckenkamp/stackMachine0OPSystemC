
#include "systemc"
#include <iostream>
#include "Stack.h"
#include "DataStack.h"
#include "Rom.h"
#include "DSController.h"
#include "Fetch.h"
#include "Alu.h"
#include "Ram.h"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

using namespace std;

/**
 * Módulo de sistema, contendo todos os demais módulos do projeto,
 * bem como suas ligações.
 */
SC_MODULE(Top) {
private:

	Stack			*rs;
	Rom				*rom;
	DataStack		*ds;
	DSController	*dc;
	Fetch			*fet;
	Alu				*ula;
	Ram				*ram;

public:

	sc_in_clk 		clk;
	int				*PC, *cntExec, *cntRam;

	SC_CTOR(Top) {
		// Inicia variáveis
		cntRam		= (int*) malloc(sizeof(int));
		cntExec		= (int*) malloc(sizeof(int));
		PC			= (int*) malloc(sizeof(int));
		*PC			= 0;
		*cntExec	= 0;
		*cntRam		= 0;

		// Instancia módulos
		rs			= new Stack("Stack");
		ds			= new DataStack("DataStack");
		dc			= new DSController("DSController");
		rom			= new Rom("ROM");
		fet			= new Fetch("Fetch");
		ula			= new Alu("Alu");
		ram			= new Ram("RAM");

		fet->setPC(PC);
		ula->setPC(PC);

		fet->setCnt(cntExec);
		ram->setCnt(cntRam);

		// Ligação entre sockets
		dc->socketOutDS.bind(ds->socket);
		dc->socketOutUla(ula->socketInDS);

		fet->socketDs.bind(dc->socketIn);
		fet->socketRom.bind(rom->socket);
		fet->socketULA.bind(ula->socketInFet);
		fet->enable(clk);

		ula->socketDS.bind(ds->socketUla);
		ula->socketRAM.bind(ram->socket);
		ula->socketRS.bind(rs->socket);

	}

	~Top(){
		// Libera da memória todos os objetos e variáveis alocados
		delete rs;
		delete ds;
		delete dc;
		delete rom;
		delete fet;
		delete ula;
		delete ram;
		free(PC);
		free(cntExec);
		free(cntRam);
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

	Top *top				= new Top("top");
	sc_signal<bool> 		enable;
	enable					= true;
	top->clk(enable);
	sc_start();

	cout<<"\n\n\nExecution cycles: "<<*top->cntExec<<"\n";
	cout<<"Total RAM accesses: "<<*top->cntRam<<"\n";

	delete top;

	return 0;
}
