///@file Fetch.cpp

#include "Fetch.h"

/**
 * Construtor do módulo.
 * @param Name
 * Nome do módulo.
 */
Fetch::Fetch(sc_module_name Name) :
		sc_module(Name) {

	// A execução é sensivel ao sinal de enable
	sensitive << enable.pos();

	// Habilita a thread principal do programa
	SC_THREAD(thread_process);
}

/**
 * Inicia o contador de programa do módulo.
 * @param pc
 * Ponteiro para o contador de programa.
 * @return
 * Não há retorno.
 */
void Fetch::setPC(int *pc) {
	this->PC	= pc;
	end			= false;
}

/**
 * Utiliza o PC atual como endereço para carregar a nova instrução.
 *
 * @return
 * Não há retorno.
 */
void Fetch::transaction_Read() {

	tlm::tlm_generic_payload* 	trans 	= new tlm::tlm_generic_payload;
	tlm::tlm_command 			cmd 	= tlm::TLM_READ_COMMAND;
	sc_time 					delay 	= sc_time(10, SC_NS);
	int 						data;

	trans->set_command(cmd);
	trans->set_address(*PC);
	trans->set_data_ptr(reinterpret_cast<unsigned char*>(&data));
	trans->set_data_length(4);
	trans->set_streaming_width(4);
	trans->set_byte_enable_ptr(0);
	trans->set_dmi_allowed(false);
	trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

	// Requisita a nova instrução
	socketRom->b_transport(*trans, delay);

	// Caso ocorra um erro, a execução do sistema é encerrada
	if (trans->is_response_error()) {
		end = true;

#ifdef DEBUG_FETCH
		cout << "\n ---------------------End of Execution---------------------\n";
#endif
	}

	wait(delay);

	// Define a nova instrução
	instruction = data;

#ifdef DEBUG_FETCH
	cout << "Fetched instruction:  " <<hex<< data<< dec<< " at time " << sc_time_stamp()
			<< " delay = " << delay << endl;
#endif
}

/**
 * Repassa a instrução atual para os demais módulos.
 * Também é responsável por detectar o fim de execução.
 *
 * @return
 * Não há retorno.
 */
void Fetch::transaction_Write() {

	tlm::tlm_generic_payload*	trans 	= new tlm::tlm_generic_payload;
	tlm::tlm_command 			cmd 	= tlm::TLM_WRITE_COMMAND;
	sc_time 					delay	= sc_time(10, SC_NS);
	int 						data	= instruction;

	trans->set_command(cmd);
	trans->set_address(1);
	trans->set_data_ptr(reinterpret_cast<unsigned char*>(&data));
	trans->set_data_length(4);
	trans->set_streaming_width(4);
	trans->set_byte_enable_ptr(0);
	trans->set_dmi_allowed(false);
	trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

	// Informa ao módulo DSController se é necessário o carregamento de operandos
	if (((int) instruction[31]) == 1) {
		trans->set_address(0);
	}

	// Realiza a transação com o módulo DSController
	socketDs->b_transport(*trans, delay);

	if (trans->is_response_error())
		SC_REPORT_ERROR("TLM-2", "Response error from b_transport");

	wait(delay);

	// envia instrução para ula
	socketULA->b_transport(*trans, delay);

	if (trans->is_response_error())
		SC_REPORT_ERROR("TLM-2", "Response error from b_transport");

	wait(delay);

}

/**
 * @brief Thread principal do sistema
 *
 * Thread responsável por ler a memória ROM e repassar a instrução carregada para
 * os demais módulos. Também é responsável por detectar o fim de execução.
 *
 * @return
 * Não há retorno.
 */
void Fetch::thread_process() {

	while (true) {


		// Incrementa o total de ciclos
		*this->cntExec = *this->cntExec + 1;

		// Lê a instrução referente ao PC atual
		transaction_Read();

		// Detecta o fim da execução
		if(end)
			break;

		// Repassa a instrução para os demais módulos
		transaction_Write();

#ifdef DEBUG_FETCH
		cout << "\nNew PC: " << *this->PC << "\n";
		cout << "\n-------------------------------------------------------------\n";
#endif
	}
}

/**
 * Inicia o contador de ciclos de execução do módulo.
 * @param cnt
 * Ponteiro para o contador de ciclos de execução.
 * @return
 * Não há retorno.
 */
void Fetch::setCnt(int *cnt){
	this->cntExec = cnt;
}

Fetch::~Fetch(){

}

