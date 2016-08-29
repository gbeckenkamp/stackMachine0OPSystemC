/// @file Rom.cpp
#include "Rom.h"

/**
 * Construtor do módulo.
 * @param Name
 * Nome do módulo.
 */
Rom::Rom(sc_module_name Name) :
		sc_module(Name) {

	// Carrega os dados na memória
	fh = new FileHandler(PATH);
	fh->LoadProgram(mem);
	free(fh);

	// Inicia o socket
	socket.register_b_transport(this, &Rom::b_transport);
}

/**
 * Socket responsável por receber requerimentos de leitura do módulo Fetch.
 * @param trans
 * Payload contendo os dados do pacote recebido.
 * @param delay
 * Timeout para a condirmação do comando recebido.
 * @return
 * Não há retorno.
 */
void Rom::b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) {

	tlm::tlm_command 		cmd = trans.get_command();
	sc_dt::uint64 			adr = trans.get_address();
	unsigned char* 			ptr = trans.get_data_ptr();
	unsigned int 			len = trans.get_data_length();
	unsigned char* 			byt = trans.get_byte_enable_ptr();
	unsigned int 			wid = trans.get_streaming_width();

	if (adr >= sc_dt::uint64(SIZE) || byt != 0 || len > 4 || wid < len)
		SC_REPORT_ERROR("TLM-2",
				"Target does not support given generic payload transaction");

	// Somente aceita leitura
	if (cmd == tlm::TLM_READ_COMMAND) {
		// Caso a instrução seja 0 ou o endereço requisitado esteja fora do range da memória, a instrução é um NOP
		if (mem[adr] == 0 || (adr<0 && adr>ROM_DEPTH)) {
			// Retorna um erro para terminar a execução
			trans.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);

		} else {

			// Prenche o campo de dados da transação com a instrução carregada
			int aux = mem[adr].value();
			memcpy(ptr, &aux, len);

			// Termina com sucesso a transação
			trans.set_response_status(tlm::TLM_OK_RESPONSE);
		}
#ifdef DEBUG_ROM
		cout << "ROM address " << adr << " being loaded. \n";
#endif
	}
}

/**
 * Destrutor do módulo.
 */
Rom::~Rom(){

}
