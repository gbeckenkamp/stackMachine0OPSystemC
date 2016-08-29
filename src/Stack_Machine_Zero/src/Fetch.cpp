/// @file Fetch.cpp
#include "Fetch.h"

/**
 * Construtor do módulo.
 * @param Name
 * Nome do módulo.
 */
Fetch::Fetch(sc_module_name Name) : sc_module(Name){

	SC_THREAD(thread_process);
	sensitive << enable.pos();

}

/**
 * Inicia o contador de programa do módulo.
 * @param pc
 * Ponteiro para o contador de programa.
 * @return
 * Não há retorno.
 */
void Fetch::setPC(int *pc) {
	this->PC = pc;
}

/**
 * Efetua uma leitura no módulo ROM.
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

	// Envia a transação para o módulo ROM
	socketRom->b_transport(*trans, delay);

	// Caso a transação de erro, encerra a execução
	if (trans->is_response_error()) {
		end = true;

#ifdef DEBUG_FETCH
		cout << "\n ---------------------End of Execution---------------------\n";
#endif
	}

	wait(delay);

	// Define a nova instrução
	instruction = data;

	unsigned i = instruction;

#ifdef DEBUG_FETCH
	cout << "Fetched instruction:  " <<hex<< i <<dec << " at time " << sc_time_stamp()
			<< " delay = " << delay << endl;
#endif
}

/**
 * Envia a nova instrução para o módulo Alu.
 *
 * @return
 * Não há retorno.
 */
void Fetch::transaction_Write() {

	tlm::tlm_generic_payload* 	trans 	= new tlm::tlm_generic_payload;
	tlm::tlm_command 			cmd 	= tlm::TLM_WRITE_COMMAND;
	sc_time 					delay 	= sc_time(10, SC_NS);
	int 						data;

	trans->set_command(cmd);
	trans->set_address(1);
	trans->set_data_ptr(reinterpret_cast<unsigned char*>(&data));
	trans->set_data_length(4);
	trans->set_streaming_width(4);
	trans->set_byte_enable_ptr(0);
	trans->set_dmi_allowed(false);
	trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

	data = instruction;

	// Envia instrução para ula
	socketULA->b_transport(*trans, delay);

	if (trans->is_response_error())
		SC_REPORT_ERROR("TLM-2", "Response error from b_transport");

	wait(delay);

}

/**
 * @brief Thread principal.
 * Thread responsável por ler novas instruções e envialas para os demais módulos.
 *
 * @return
 * Não há retorno.
 */
void Fetch::thread_process() {

	while (true) {

		*this->cntExec = *this->cntExec + 1;
		// Lê a nova instrução.
		transaction_Read();

		// Termina a execução após um Nop ou um erro
		if(end)
			break;

		// Envia a nova instrução para a Alu
		transaction_Write();

#ifdef DEBUG_FETCH
		cout << "\n-------------------------------------------------------------\n";
		cout << "\nPC: " << *PC << "\n";
#endif
	}
}

/**
 * Inicia o ponteiro contador de ciclos de execução.
 *
 * @return
 * Não há retorno.
 */
void Fetch::setCnt(int *cnt){
	this->cntExec = cnt;
}

Fetch::~Fetch(){

}
