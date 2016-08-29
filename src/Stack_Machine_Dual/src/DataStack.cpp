///@file DataStack.cpp

#include "DataStack.h"

/**
 * Construtor do módulo.
 * @param Name
 * Nome do módulo.
 */
DataStack::DataStack(sc_module_name Name) :
		sc_module(Name) {
	// Endereço inicial do topo da pilha
	dsp = 0;
	// Inicia os sockets
	socket.register_b_transport(this, &DataStack::b_transport);
	socketUla.register_b_transport(this, &DataStack::b_transport);
}

/**
 * Socket responsável por fazer a comunicação com o módulo DSController.
 * @param trans
 * Payload contendo os dados do pacote recebido.
 * @param delay
 * Timeout para a condirmação do comando recebido.
 * @return
 * Não há retorno.
 */
void DataStack::b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) {

	tlm::tlm_command	cmd = trans.get_command();
	sc_dt::uint64		adr = trans.get_address();
	unsigned char*		ptr = trans.get_data_ptr();
	unsigned int		len = trans.get_data_length();
	unsigned char*		byt = trans.get_byte_enable_ptr();
	unsigned int		wid = trans.get_streaming_width();

	if (adr >= sc_dt::uint64(SIZE) || byt != 0 || len > 4 || wid < len)
		SC_REPORT_ERROR("TLM-2",
				"Target does not support given generic payload transaction");

	// Caso o comando recebido seja de leitura, repassa o dado requisitado para o DSController
	if (cmd == tlm::TLM_READ_COMMAND) {

		int relative_position = 0;

		// Pega o endereço do dado no topo da pilha
		_dsp = dsp - 1;

		// Pega a posição relativa do endereço requisitado, visto que a pilha é circular
		if ((int) adr > _dsp) {
			relative_position = DS_SIZE - (adr - _dsp);
		} else {
			relative_position = _dsp - adr;
		}

		// Substitui-se o campo de dados da transação com o dado lido
		int aux = RS[relative_position];
		memcpy(ptr, &aux, len);

#ifdef DEBUG_DS
		cout<<"Relative position: "<<relative_position<<" value "<<RS[relative_position]<<"\n";
#endif

	}
	// Caso o comando recebido seja de escrita, aloca o dado no topo da pilha
	else if (cmd == tlm::TLM_WRITE_COMMAND) {

		// Se o topo da pilha está na última posição do vetor, direciona o endereço de topo da pilha para o inicio do vetor
		if (dsp == DS_SIZE) {
			dsp = _dsp = 0;
		}

		// Calcula o novo endereço de topo da pilha
		_dsp = dsp + 1;

		// Lê o campo de dados da transação e aloca no topo da pilha
		int i;
		memcpy(&i, ptr, len);
		RS[dsp] = i;
		dsp = _dsp;

#ifdef DEBUG_DS
		cout << "Stack pointer: " << dsp << "\n";
		for (int x = 0; x < dsp; x++) {
			cout << (int) RS[x] << ", ";
		}
		cout << "\n";
#endif
	}

	// Termina a transação
	trans.set_response_status(tlm::TLM_OK_RESPONSE);

}

/**
 * Destrutor do módulo.
 */
DataStack::~DataStack(){

}

