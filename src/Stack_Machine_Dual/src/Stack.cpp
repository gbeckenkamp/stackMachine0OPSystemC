///@file Stack.cpp

#include "Stack.h"

/**
 * Construtor do módulo.
 * @param Name
 * Nome do módulo.
 */
Stack::Stack(sc_module_name Name) :
		sc_module(Name) {

	// Inicia o endereço de topo da pilha
	rsp = 0;

	// Inicia o socket
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

	tlm::tlm_command 		cmd = trans.get_command();
	sc_dt::uint64 			adr = trans.get_address();
	unsigned char* 			ptr = trans.get_data_ptr();
	unsigned int 			len = trans.get_data_length();
	unsigned char* 			byt = trans.get_byte_enable_ptr();
	unsigned int 			wid = trans.get_streaming_width();

	if (adr >= sc_dt::uint64(SIZE) || byt != 0 || len > 4 || wid < len)
		SC_REPORT_ERROR("TLM-2",
				"Target does not support given generic payload transaction");

	// Caso o comando seja de leitura
	if (cmd == tlm::TLM_READ_COMMAND) {
		// Endereço 1 requer o tamanho da pilha
		if (adr == 1) {
			memcpy(ptr, &rsp, len);
		} else if (rsp != 0) {

			// Atualiza _rsp
			this->_rsp = this->rsp;
			this->_rsp--;
			// Passa para o campo de dados da transação o valor no topo da pilha
			int i = RS[this->_rsp];
			memcpy(ptr, &i, len);
			// pop
			this->rsp = this->_rsp;

		}else{
			cout<<"\nWARNING return stack underflow.\n";
		}

	}
	// Caso o comando seja de escrita
	else if (cmd == tlm::TLM_WRITE_COMMAND) {

		// O topo da pilha não atingiu o máximo do vetor
		if (rsp != RS_SIZE) {
			// Atualiza _rsp
			this->_rsp = this->rsp;
			// Passa o campo de dados da transação para o topo da pilha
			int i;
			memcpy(&i, ptr, len);
			RS[rsp] = i;
			// push
			this->_rsp++;
			this->rsp = this->_rsp;

		}else{
			cout<<"\nWARNING return stack overflow.\n";
		}

	}
	// Termina com sucesso a transação
	trans.set_response_status(tlm::TLM_OK_RESPONSE);

#ifdef DEBUG_RS
	cout << "Return stack pointer: " << rsp << "\n";


	for (int x = 0; x < rsp; x++) {
		cout << (int) RS[x] << ", ";
	}
#endif
}

/**
 * Destrutor do módulo Alu.
 */
Stack::~Stack(){

}

