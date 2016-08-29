///@file DataStack.cpp
#include "DataStack.h"

/**
 * Construtor do módulo.
 * @param Name
 * Nome do módulo.
 */
DataStack::DataStack(sc_module_name Name) :
		sc_module(Name) {

	sp = 0;
	socket.register_b_transport(this, &DataStack::b_transport);
}

/**
 * Socket responsável por gerenciar e fazer a interface da pilha de dados com os demais módulos.
 * @param trans
 * Payload contendo os dados do pacote recebido.
 * @param delay
 * Timeout para a condirmação do comando recebido.
 * @return
 * Não há retorno.
 */
void DataStack::b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) {

	tlm::tlm_command 	cmd = trans.get_command();
	sc_dt::uint64 		adr = trans.get_address();
	unsigned char*		ptr = trans.get_data_ptr();
	unsigned int 		len = trans.get_data_length();
	unsigned char* 		byt = trans.get_byte_enable_ptr();
	unsigned int 		wid = trans.get_streaming_width();

	if (adr >= sc_dt::uint64(SIZE) || byt != 0 || len > 4 || wid < len)
		SC_REPORT_ERROR("TLM-2",
				"Target does not support given generic payload transaction");

	// Ao executar uma leitura, lê o topo da pilha
	if (cmd == tlm::TLM_READ_COMMAND) {

		if (adr >= 0 && adr < 4) {

			memcpy(ptr, &DS[sp], len);

			updateBuffers();
		}
	}

	// Ao executar uma escrita, aloca o dados em um dos 4 buffers do topo da pilha de dados
	else if (cmd == tlm::TLM_WRITE_COMMAND) {

		if (adr >= 0 && adr < 4) {

			int i;
			memcpy(&i, ptr, len);
			DS[sp - adr - 1] = i;

			updateBuffers();
		}

	// Ao executar um comando ignorado, realiza uma modulação na pilha de dados ou uma
	//	sincronização dos buffers com o topo da pilha
	} else if (cmd == tlm::TLM_IGNORE_COMMAND) {

		if (adr == 1) {				// push
			sp++;
		} else if (adr == 0) {		// pop
			sp--;
		} else if (adr == 2) {		// push + sync dos buffers
			sp++;
			updateBuffers();
		} else if (adr == 3) {		// pop + sync dos buffers
			sp--;
			updateBuffers();
		} else {					// sync dos buffers
			updateBuffers();
		}

		if(sp > DS_SIZE){
			cout<< "Stack overflow\n";
			sp = DS_SIZE;
		}else if(sp < 0){
			cout<< "Stack underflow\n";
			sp = 0;
		}

	}
	// Termina a transação com sucesso
	trans.set_response_status(tlm::TLM_OK_RESPONSE);
#ifdef DEBUG_DS
	cout << "Stack pointer: " << sp << "\n";

	cout << "Data stack: ";
	for (int x = 0; x < sp; x++) {
		cout << (int) DS[x] << ", ";
	}
	cout << "\nDS buffers: ";
	for (int x = 0; x < 4; x++) {
		cout << (int) buffersDS[x] << ",";
	}
	cout << "\n";
#endif
}

/**
 * Executa a sincronização dos buffers com o topo da pilha.
 *
 * @return
 * Não há retorno.
 */
void DataStack::updateBuffers() {

	int x = sp, y = 0;
	while (x > 0 && y < 4) {

		buffersDS[y] = DS[sp - y - 1];
		x--;
		y++;
	}
}

/**
 * Inicia os buffers de topo de pilha.
 *
 * @return
 * Não há retorno.
 */
void DataStack::setBuffers(int *buf) {
	this->buffersDS = buf;
}

/**
 * Destrudor do módulo.
 */
DataStack::~DataStack(){

}
