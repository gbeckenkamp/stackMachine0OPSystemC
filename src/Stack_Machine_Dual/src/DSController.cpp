///@file DSController.cpp

#include "DSController.h"

/**
 * Construtor do módulo.
 * @param Name
 * Nome do módulo.
 */
DSController::DSController(sc_module_name Name) :
		sc_module(Name) {
	search = false;

	// Register callback for incoming b_transport interface method call
	socketIn.register_b_transport(this, &DSController::b_transport);
	//SC_THREAD(thread_process);
}

/**
 * Socket responsável por receber a instrução carregada pelo Fetch.
 * @param trans
 * Payload contendo os dados do pacote recebido.
 * @param delay
 * Timeout para a condirmação do comando recebido.
 * @return
 * Não há retorno.
 */
void DSController::b_transport(tlm::tlm_generic_payload& trans,
		sc_time& delay) {

	tlm::tlm_command		cmd = trans.get_command();
	sc_dt::uint64			adr = trans.get_address();
	unsigned char*			ptr = trans.get_data_ptr();
	unsigned int			len = trans.get_data_length();
	unsigned char*			byt = trans.get_byte_enable_ptr();
	unsigned int			wid = trans.get_streaming_width();

	if (adr >= sc_dt::uint64(SIZE) || byt != 0 || len > 4 || wid < len)
		SC_REPORT_ERROR("TLM-2",
				"Target does not support given generic payload transaction");

	// Caso o comando seja uma escrita, atualiza a instrução contida no módulo
	if (cmd == tlm::TLM_WRITE_COMMAND) {

		int i;
		load = adr;
		memcpy(&i, ptr, len);
		instruction = i;

		// Habilita a flag que avisa a chegada de uma nova instrução
		search = true;

	}

	// Termina a transação
	trans.set_response_status(tlm::TLM_OK_RESPONSE);

	// Inicia o tratamento da nova instrução
	thread_process();
}

/**
 * Processo responsável por interpretar a instrução e carregar os
 * operandos necessários, que são repassados ao módulo Alu.
 * @return
 * Não há retorno.
 */
void DSController::thread_process() {

	// Caso a instrução seja nova
	if (search) {
		// Caso load seja 1, é necessário análisar a instrução
		if (load == 1) {

			// O bit 28 da instrução indica o carregamento de um único operando
			if ((int) instruction[28] == 1) {

				if ((int) instruction.range(30, 29) == 3) {				//ALU - '11'

					/* Este teste é feito pois no caso de uma instrução carregando
					 * o tamanho da pilha de retorno é a única que utiliza os bits
					 * 11 e 20 com valor 1 e  que não precisa carregar nenhum operando */
					if ((int) instruction[11] == 0) {

						make_Transaction(instruction.range(7, 4), 0);	//OP1
#ifdef DEBUG_DSC
						cout << " ALU, 1 operand\n";
#endif
					}
				} else if ((int) instruction.range(30, 29) == 1) {		//branch - '01'
#ifdef DEBUG_DSC
					cout << "Jump or call\n";
#endif
				} else if ((int) instruction.range(30, 29) == 2) {		//store - '10'
					// Se usa um operando como endereço
					if (instruction[27] == 1) {

						make_Transaction(instruction.range(7, 4), 0);	//OP1
						make_Transaction(instruction.range(3, 0), 1);	//OP1
#ifdef DEBUG_DSC
						cout << " Store, 2 operand\n";
#endif
					} else {

						make_Transaction(instruction.range(7, 4), 0);	//OP1

#ifdef DEBUG_DSC
						cout << " Store, 1 operand\n";
#endif
					}
				}

			} else {

				if ((int) instruction.range(30, 29) == 3) {				//ALU - '11'

					make_Transaction(instruction.range(7, 4), 0);		//OP1
					make_Transaction(instruction.range(3, 0), 1);		//OP2
#ifdef DEBUG_DSC
					cout << " ALU, 2 operand\n";
#endif
				} else if ((int) instruction.range(30, 29) == 1) {		//branch - '01'

					make_Transaction(instruction.range(7, 4), 0);		//OP1
					make_Transaction(instruction.range(3, 0), 1);		//OP2
#ifdef DEBUG_DSC
					cout << " Branch, 2 operand\n";
#endif
				} else if ((int) instruction.range(30, 29) == 2) {		//load - '10'

					// Se usa um operando como endereço
					if (instruction[27] == 1) {

						make_Transaction(instruction.range(7, 4), 0);	//OP1
#ifdef DEBUG_DSC
						cout << " Load, 1 operand\n";
#endif
					} else {
#ifdef DEBUG_DSC
						cout << " Load\n";
#endif
					}

				} else if ((int) instruction.range(30, 29) == 1) {

					make_Transaction(instruction.range(7, 4), 0);		//OP1
					make_Transaction(instruction.range(3, 0), 1);		//OP2
				}
			}
		}

		// Operandos da instrução foram carregados
		search = false;

		// Termina a passagem de operandos ao módulo Alu
		make_end();
#ifdef DEBUG_DSC
		cout << "All operands ready.\n";
#endif
	}
}

/**
 * Envia uma transação para o módulo Alu com o endereço de buffer 3, forçando assim o fim da transmissão dos operandos.
 * @return
 * Não há retorno.
 */
void DSController::make_end() {

	tlm::tlm_generic_payload*	trans	= new tlm::tlm_generic_payload;
	tlm::tlm_command 			cmd		= tlm::TLM_WRITE_COMMAND;
	sc_time						delay	= sc_time(10, SC_NS);
	int 						data;

	trans->set_command(cmd);
	trans->set_address(3);
	trans->set_data_ptr(reinterpret_cast<unsigned char*>(&data));
	trans->set_data_length(4);
	trans->set_streaming_width(4); // = data_length to indicate no streaming
	trans->set_byte_enable_ptr(0); // 0 indicates unused
	trans->set_dmi_allowed(false); // Mandatory initial value
	trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE); // Mandatory initial value

	// Realiza a transação com o módulo Ula
	socketOutUla->b_transport(*trans, delay);

	if (trans->is_response_error())
		SC_REPORT_ERROR("TLM-2", "Response error from b_transport");

	wait(delay);

}

/**
 * Lê o registrador temporal da pilha de dados e envia o operando para o módulo Alu,
 * informando em qual posição do buffer contido na Alu deve ser guardado o operando carregado.
 * @param addr
 * Endereço do registrador temporal especificado pelo operando.
 * @param buffer_position
 * Endereço do buffer contido na Alu onde o dado carregado da pilha será copiado.
 */
void DSController::make_Transaction(int addr, int buffer_position) {

	tlm::tlm_generic_payload*	trans 	= new tlm::tlm_generic_payload;
	sc_time						delay 	= sc_time(10, SC_NS);
	tlm::tlm_command 			cmd 	= tlm::TLM_READ_COMMAND;
	int 						data;

	trans->set_command(cmd);
	trans->set_address(addr);
	trans->set_data_ptr(reinterpret_cast<unsigned char*>(&data));
	trans->set_data_length(4);
	trans->set_streaming_width(4);
	trans->set_byte_enable_ptr(0);
	trans->set_dmi_allowed(false);
	trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

	// Lê da pilha de dados o operador necessário
	socketOutDS->b_transport(*trans, delay);

	if (trans->is_response_error())
		SC_REPORT_ERROR("TLM-2", "Response error from b_transport");

	wait(delay);

#ifdef DEBUG_DSC
	cout << "Data read: " << data << "\n";
#endif

	// Reutiliza o payload, substituindo o comando por leitura e o endereço pelo endereço do buffer
	trans->set_command(tlm::TLM_WRITE_COMMAND);
	trans->set_address(buffer_position);

	// Realiza a nova transação com o módulo Alu
	socketOutUla->b_transport(*trans, delay);

	if (trans->is_response_error())
		SC_REPORT_ERROR("TLM-2", "Response error from b_transport");

	wait(delay);

}

/**
 * Destrutor do módulo.
 */
DSController::~DSController(){

}

