/// \file Ram.cpp
#include "Ram.h"

/**
 * Construtor do módulo.
 * @param Name
 * Nome do módulo.
 */
Ram::Ram(sc_module_name Name) : sc_module(Name){

	socket.register_b_transport(this, &Ram::b_transport);

}

/**
 * Socket responsável por mater e fazer a interface com a memórai RAM.
 * @param trans
 * Payload contendo os dados do pacote recebido.
 * @param delay
 * Timeout para a condirmação do comando recebido.
 * @return
 * Não há retorno.
 */
void Ram::b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) {

	tlm::tlm_command 	cmd = trans.get_command();
	sc_dt::uint64 		adr = trans.get_address();
	unsigned char* 		ptr = trans.get_data_ptr();
	unsigned int 		len = trans.get_data_length();
	unsigned char* 		byt = trans.get_byte_enable_ptr();
	unsigned int 		wid = trans.get_streaming_width();

	if (adr >= sc_dt::uint64(SIZE) || byt != 0 || len > 4 || wid < len)
		SC_REPORT_ERROR("TLM-2",
				"Target does not support given generic payload transaction");

	// Efetua a leitura do endereço especificado
	if (cmd == tlm::TLM_READ_COMMAND) {

		int aux = mem[adr];
		memcpy(ptr, &aux, len);

		*this->cntRamAcc = *this->cntRamAcc + 1;

	// Escreve o dado recebido no endereço especificado
	} else if (cmd == tlm::TLM_WRITE_COMMAND) {

		int i ;
		memcpy(&i, ptr, len);
		mem[adr] = i;

		*this->cntRamAcc = *this->cntRamAcc + 1;

#ifdef DEBUG_RAM
		cout<<"\nWriting value: "<<i<<"\n";
#endif
	}

	// Termina a transação com sucesso
	trans.set_response_status(tlm::TLM_OK_RESPONSE);

#ifdef DEBUG_RAM
	cout << "\nActual RAM block: ";
	for (int x = DEBUG_PRINT_RAM_DOWN; x < DEBUG_PRINT_RAM_TOP; x++) {
		cout << mem[x] << ", ";
	}
	cout << "\n";
#endif
}

/**
 * Inicia o ponteiro contador de acessos a memória RAM.
 * @param acc
 * Ponteiro para o contador de acessos.
 * @return
 * Não há retorno.
 */
void Ram::setRamAcc(int *acc){
	this->cntRamAcc = acc;
}

/**
 * Destrutor do módulo.
 */
Ram::~Ram(){

}
