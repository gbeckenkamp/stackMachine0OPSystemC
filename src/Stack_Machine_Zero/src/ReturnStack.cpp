/// @file ReturnStack.cpp

#include "ReturnStack.h"

/**
 * Construtor do módulo.
 * @param Name
 * Nome do módulo.
 */
Stack::Stack(sc_module_name Name) :
		sc_module(Name) {

	sp = 0;
	socket.register_b_transport(this, &Stack::b_transport);
}

/**
 * Socket responsável por receber requisições oriundas da Alu.
 * @param trans
 * Payload contendo os dados do pacote recebido.
 * @param delay
 * Timeout para a condirmação do comando recebido.
 * @return
 * Não há retorno.
 */
void Stack::b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) {

	tlm::tlm_command 	cmd = trans.get_command();
	sc_dt::uint64 		adr = trans.get_address();
	unsigned char* 		ptr = trans.get_data_ptr();
	unsigned int 		len = trans.get_data_length();
	unsigned char* 		byt = trans.get_byte_enable_ptr();
	unsigned int 		wid = trans.get_streaming_width();

	if (adr >= sc_dt::uint64(SIZE) || byt != 0 || len > 4 || wid < len)
		SC_REPORT_ERROR("TLM-2",
				"Target does not support given generic payload transaction");

	// Caso o comando seja de leitura, retorna o topo da pilha
	if (cmd == tlm::TLM_READ_COMMAND) {

		if (adr >= 0 && adr < 4) {

			memcpy(ptr, &RS[sp], len);
			updateBuffers();
		}
	}

	// Caso seja um comando de escrita, aloca o dado em um dos buffers ao topo da pilha
	else if (cmd == tlm::TLM_WRITE_COMMAND) {

		if (adr >= 0 && adr < 4) {

			int i;
			memcpy(&i, ptr, len);
			RS[sp - adr - 1] = i;

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

	updateBuffers();

#ifdef DEBUG_RS
	cout << "Stack pointer: " << sp << "\n";

	for (int x = 0; x < sp; x++) {
		cout << (int) RS[x] << ", ";
	}
#endif

}

/**
 * Executa a sincronização dos buffers com o topo da pilha.
 *
 * @return
 * Não há retorno.
 */
void Stack::updateBuffers() {
	int x = sp, y = 0;
	while (x > 0 && y < 4) {
		buffersRS[y] = RS[sp - y - 1];
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
void Stack::setBuffers(int *buf) {
	this->buffersRS = buf;
}

/**
 * Destrudor do módulo.
 */
Stack::~Stack(){

}
