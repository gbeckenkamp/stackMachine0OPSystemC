///@file Alu.cpp

#include "Alu.h"

/**
 * Construtor do módulo.
 * @param Name
 * Nome do módulo.
 */
Alu::Alu(sc_module_name Name) :
		sc_module(Name) {

	// Inicia as flags e os sockets
	fet_ready = buff_ready = false;
	socketInFet.register_b_transport(this, &Alu::b_transport_Fet);
	socketInDS.register_b_transport(this, &Alu::b_transport_DS);
}

/**
 * Inicia o contador de programa do módulo.
 * @param pc
 * Ponteiro para o contador de programa.
 * @return
 * Não há retorno.
 */
void Alu::setPC(int *pc) {
	this->PC = pc;
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
void Alu::b_transport_Fet(tlm::tlm_generic_payload& trans, sc_time& delay) {

	// Aloca em variáveis locais os parametros recebidos
	tlm::tlm_command cmd	= trans.get_command();
	sc_dt::uint64 adr		= trans.get_address();
	unsigned char* ptr		= trans.get_data_ptr();
	unsigned int len		= trans.get_data_length();
	unsigned char* byt		= trans.get_byte_enable_ptr();
	unsigned int wid		= trans.get_streaming_width();

	if (adr >= sc_dt::uint64(SIZE) || byt != 0 || len > 4 || wid < len)
		SC_REPORT_ERROR("TLM-2",
				"Target does not support given generic payload transaction");

	// Trata o recebimento de transação
	if (cmd == tlm::TLM_WRITE_COMMAND) {
		int i;
		memcpy(&i, ptr, len);
		instruction			= i;

		// Instrução já foi carregada e o módulo pode execurar suas tarefas
		fet_ready			= true;

	}
	trans.set_response_status(tlm::TLM_OK_RESPONSE);

	// Inicia a execução
	if (buff_ready) {
		compute();
	}

}

/**
 * Socket responsável por receber os dados carregados da pilha de dados.
 * @param trans
 * Payload contendo os dados do pacote recebido.
 * @param delay
 * Timeout para a condirmação do comando recebido.
 * @return
 * Não há retorno.
 */
void Alu::b_transport_DS(tlm::tlm_generic_payload& trans, sc_time& delay) {

	tlm::tlm_command cmd	= trans.get_command();
	sc_dt::uint64 adr		= trans.get_address();
	unsigned char* ptr		= trans.get_data_ptr();
	unsigned int len		= trans.get_data_length();
	unsigned char* byt		= trans.get_byte_enable_ptr();
	unsigned int wid		= trans.get_streaming_width();

	if (adr >= sc_dt::uint64(SIZE) || byt != 0 || len > 4 || wid < len)
		SC_REPORT_ERROR("TLM-2",
				"Target does not support given generic payload transaction");

	// Trata o recebimento da transação
	if (cmd == tlm::TLM_WRITE_COMMAND) {

		/* O máximo de operandos que podem ser carregados é três. Enquanto o endereço
		recebido for menor que três, o módulo aloca o dado recebido no buffer*/
		if (adr == 3) {
			buff_ready = true;
		} else {
			int i;
			memcpy(&i, ptr, len);
			buffers[adr]		= i;
		}

	}
	trans.set_response_status(tlm::TLM_OK_RESPONSE);

	// Inicia a execução
	if (fet_ready) {
		compute();
	}

}

/**
 * Interpreta a instrução atual e realiza as computações necessárias.
 * @return
 * Não há retorno.
 */
void Alu::compute() {

	if (instruction[31] == 1) {						// O bit 31 da instrução sempre indica um literal

		// Aloca o valor contido na instrução na pilha de dados
		writeDS((int) instruction.range(30, 0));
		fet_ready = buff_ready = false;

		*this->PC = *this->PC + 1;

	} else {

		if (fet_ready == true && buff_ready == true) {

			/*
			 * Caso o bit 31 da instrução seja 0, deve-se testar os bits 30 e 29 para
			 * definir o tipo de instrução
			 */

			switch ((int) instruction.range(30, 29)) {
			case 0:									//'00'
				/*
				 *	NOP, fim da execução
				 */
#ifdef DEBUG_ALU
				cout
				<< "\n ---------------------End of Execution---------------------";
#endif

				break;
			case 1:									//'01'
				/*
				 *	Salto
				 *
				 * O bit 28 indica a não condicionalidade da instrução
				 */
				if (instruction[28] == 1) {

					if (instruction[8] == 1) {

						/*
						 * Chamada, o PC é guardado no topo da pilha de retorno e substituido pelo
						 * endereço especificado
						 */
   						writeRS(*PC);
						*PC = (int) instruction.range(27, 11);

#ifdef DEBUG_ALU
						cout << "\n---------------\nCall\n---------------\n";
#endif

					} else {

						// Salto incondicional
						*PC = (int) instruction.range(27, 11);
#ifdef DEBUG_ALU
						cout << "\n---------------\nJump\n---------------\n";
#endif
					}
				} else {

					// Salto ou chamada condicional
#ifdef DEBUG_ALU
					cout << "\n---------------\nBranch\n---------------\n";
#endif

					int pc			= *this->PC;
					bool flagCall	= false;

					// Testa qual o tipo de condição
					switch ((int) instruction.range(10, 9)) {

					case 0: 						//'00' op1 == op2

						if (buffers[0] == buffers[1]) {

							*this->PC = (int) instruction.range(27, 11);
							flagCall = true;

						} else {

							*this->PC = *this->PC + 1;

						}
						break;

					case 1: 						//'01' op1 != op2
						if (buffers[0] != buffers[1]) {

							*this->PC = (int) instruction.range(27, 11);
							flagCall = true;

						} else {

							*this->PC = *this->PC + 1;

						}
						break;

					case 2: 						//'10' op1 >= op2
						if (buffers[0] > buffers[1]) {

							*this->PC = (int) instruction.range(27, 11);
							flagCall = true;

						} else {

							*this->PC = *this->PC + 1;

						}
						break;

					case 3: 						//'11' op1 <= op2
						if (buffers[0] >= buffers[1]) {

							*this->PC = (int) instruction.range(27, 11);
							flagCall = true;

						} else {

							*this->PC = *this->PC + 1;

						}
						break;
					}

					//  Em caso de uma chamada, o PC atual é colocado na pilha de retorno
					if (instruction[8] == 1 && flagCall == true) {

						writeRS(pc);

					}

				}

				break;
			case 2:									//'10'
				/*
				 *	RAM
				 */
#ifdef DEBUG_ALU
				cout << "\n---------------\nRAM\n---------------\n";
#endif

				// O bit 28 informa se a operação de memória é um load ou um store
				if (instruction[28] == 0) {			//Load
					if (instruction[27] == 1) {		// O endereço está contido na pilha de dados

						readRAM(buffers[0]);
#ifdef DEBUG_ALU
						cout << "Loading from address " << buffers[0] << ".\n";
#endif
					} else {						// O endereço está contido no offset da isntrução

						readRAM((int) instruction.range(26, 8));
#ifdef DEBUG_ALU
						cout << "Loading from address "
						<< (int) instruction.range(26, 8) << ".\n";
#endif
					}

					*this->PC = *this->PC + 1;		// Avança o PC

				} else {							//Store
					if (instruction[27] == 1) {		// O endereço está contido na pilha de dados

						writeRAM((int) buffers[1], (int) buffers[0]);
#ifdef DEBUG_ALU
						cout << "Storing temporal register "
						<< (int) instruction.range(7, 4)
						<< " into the address " << buffers[1] << ".\n";
#endif
					} else {						// O endereço está contido no offset da isntrução

						writeRAM((int) instruction.range(26, 8),
														(int) buffers[0]);
#ifdef DEBUG_ALU
						cout << "Storing temporal register "
						<< (int) instruction.range(7, 4)
						<< " into the address "
						<< (int) instruction.range(26, 8) << ".\n";
#endif
					}

					*this->PC = *this->PC + 1;		// Avança PC
				}

				break;
			case 3:									//'11'
				/*
				 *	ULA
				 */
#ifdef DEBUG_ALU
				cout << "\n=============\nULA\n=============\n";
#endif
				// Monta o campo option da instrução, concatenando o bit 28 ao resto do campo opt
				int opt				= (int) instruction.range(14, 8);
				opt					= opt << 1;
				opt					= opt | instruction[28];

				// Testa o option da instrução
				switch (opt) {

				case 0:				// ADD - '00000'

					writeDS((int) (buffers[0] + buffers[1]));
#ifdef DEBUG_ALU
				cout << "ADD( " << buffers[0] << " , " << buffers[1]
				<< " )\n";
#endif
					break;
				case 1:				// Nothing - '00001'

					break;
				case 2:				// SUB - '00010'

					writeDS((int) (buffers[0] - buffers[1]));
#ifdef DEBUG_ALU
				cout << "SUB( " << buffers[0] << " , " << buffers[1]
				<< " )\n";
#endif
					break;
				case 3:				// Not - '00011'

					writeDS((int) (~buffers[0]));
#ifdef DEBUG_ALU
				cout << "NOT( " << buffers[0] << " )\n";
#endif
					break;
				case 4:				// AND - '00100'

					writeDS((int) (buffers[0] & buffers[1]));
#ifdef DEBUG_ALU
				cout << "AND( " << buffers[0] << " , " << buffers[1]
				<< " )\n";
#endif
					break;
				case 5:				// Shift Left - '00101'

					writeDS((int) (buffers[0] << instruction.range(27, 15)));
#ifdef DEBUG_ALU
				cout << "Left shift( " << buffers[0] << " )\n";
#endif
					break;
				case 6:				// OR - '00110'

					writeDS((int) (buffers[0] | buffers[1]));
#ifdef DEBUG_ALU
				cout << "OR( " << buffers[0] << " , " << buffers[1]
				<< " )\n";
#endif
					break;

				case 7:				// Shift Right - '00111'

					writeDS((int) (buffers[0] >> instruction.range(27, 15)));
#ifdef DEBUG_ALU
				cout << "Right shift( " << buffers[0] << " )\n";
#endif
					break;

				case 8:				// XOR - '01000'

					writeDS((int) (buffers[0] ^ buffers[1]));
#ifdef DEBUG_ALU
				cout << "XOR( " << buffers[0] << " , " << buffers[1]
				<< " )\n";
#endif
					break;

				case 9:				// ++ - '01001'

					writeDS((int) (buffers[0] + instruction.range(27, 15)));
#ifdef DEBUG_ALU
				cout << "Plus( " << buffers[0] << " , "
				<< ((int) instruction.range(27, 15)) << " )\n";
#endif
					break;

				case 10:			// XNOR - '01010'

					writeDS((int) (~(buffers[0] ^ buffers[1])));
#ifdef DEBUG_ALU
				cout << "XNOR( " << buffers[0] << " , " << buffers[1]
				<< " )\n";
#endif
					break;

				case 26:			// * - '11010'

					writeDS((int) (buffers[0] * buffers[1]));
#ifdef DEBUG_ALU
				cout << "MULT( " << buffers[0] << " , " << buffers[1]
				<< " )\n";
#endif
					break;
				case 11:			// -- - '01011'

					writeDS((int) (buffers[0] - instruction.range(27, 15)));
#ifdef DEBUG_ALU
				cout << "Less( " << buffers[0] << " , "
				<< ((int) instruction.range(27, 15)) << " )\n";
#endif
					break;

				case 12:			// / - '01100'

					writeDS((int) (buffers[0] / buffers[1]));
#ifdef DEBUG_ALU
				cout << "DIV( " << buffers[0] << " , " << buffers[1]
				<< " )\n";
#endif
					break;

				case 14:			// > - '01110'

					if (buffers[0] > buffers[1])
						writeDS(1);
					else
						writeDS(0);
#ifdef DEBUG_ALU
				cout << "Bigger( " << buffers[0] << " , " << buffers[1]
				<< " )\n";
#endif
					break;

				case 16:			// = - '10000'

					if (buffers[0] == buffers[1])
						writeDS(1);
					else
						writeDS(0);
#ifdef DEBUG_ALU
				cout << "Equal( " << buffers[0] << " , " << buffers[1]
				<< " )\n";
#endif
					break;

				case 17:

					writeDS(readRS(1));
					break;

				case 18:			// != - '10010'
					if (buffers[0] != buffers[1])
						writeDS(1);
					else
						writeDS(0);
					break;

				case 20:			// >= - '10100'

					if (buffers[0] >= buffers[1])
						writeDS(1);
					else
						writeDS(0);
#ifdef DEBUG_ALU
				cout << "BiggerE( " << buffers[0] << " , " << buffers[1]
				<< " )\n";
#endif
					break;

				case 22:			// <= - '10110'

					if (buffers[0] <= buffers[1])
						writeDS(1);
					else
						writeDS(0);
#ifdef DEBUG_ALU
				cout << "SmallerE( " << buffers[0] << " , " << buffers[1]
				<< " )\n";
#endif
					break;

				case 24:			// < - '11000'

					if (buffers[0] < buffers[1])
						writeDS(1);
					else
						writeDS(0);
#ifdef DEBUG_ALU
				cout << "Smaller( " << buffers[0] << " , " << buffers[1]
				<< " )\n";
#endif
					break;

				}

				/*
				 * O opcode contido nos bits 28 a 26 mostra o que deve ser feito com as pilhas
				 */
				switch ((int) instruction.range(14, 12)) {

				case 0:				// Nothing - '000'
					break;

				case 4:				// PC=>RS - '100'

					writeRS(*PC);
#ifdef DEBUG_ALU
				cout << "PC=>RS\n";
#endif
					break;

				case 5:				// RS=>PC - '101'

					*PC = readRS(0);
#ifdef DEBUG_ALU
				cout << "RS=>PC\n";
#endif
					break;

				case 1:				// RS=>DS - '001'

					writeDS(readRS(0));
#ifdef DEBUG_ALU
				cout << "RS=>DS\n";
#endif
					break;

				case 2:				// DS=>RS - '010'

					writeRS((int) buffers[0]);
#ifdef DEBUG_ALU
				cout << "DS=>RS\n";
#endif
					break;
				}

				*this->PC = *this->PC + 1;		//Avança PC

				break;

			}

			fet_ready = buff_ready = false;		// Reseta as flags para uma nova execução
		}

	}

}

/**
 * Executa a leitura da memória RAM.
 * @param addr
 * Endereço de memória a ser lido.
 * @return
 * Não há retorno.
 */
void Alu::readRAM(int addr) {

	tlm::tlm_generic_payload*	trans	= new tlm::tlm_generic_payload;
	tlm::tlm_command 			cmd		= tlm::TLM_READ_COMMAND;
	sc_time 					delay	= sc_time(10, SC_NS);
	int data;

	trans->set_command(cmd);
	trans->set_address(addr);
	trans->set_data_ptr(reinterpret_cast<unsigned char*>(&data));
	trans->set_data_length(4);
	trans->set_streaming_width(4);
	trans->set_byte_enable_ptr(0);
	trans->set_dmi_allowed(false);
	trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

	// Envia o comando de leitura a RAM
	socketRAM->b_transport(*trans, delay);

	if (trans->is_response_error())
		SC_REPORT_ERROR("TLM-2", "Response error from b_transport");

	wait(delay);

	// Reutiliza o payload para fazer a transação com a pilha de dados
	trans->set_command(tlm::TLM_WRITE_COMMAND);

	// Envia o dado oriundo da RAM para a pilha de dados
	socketDS->b_transport(*trans, delay);

	if (trans->is_response_error())
		SC_REPORT_ERROR("TLM-2", "Response error from b_transport");

	wait(delay);

}

/**
 * Executa a escrita de um dado na memória RAM.
 * @param addr
 * Endereço de memória a ser escrito.
 * @param op
 * Dado a ser escrito.
 * @return
 * Não há retorno.
 */
void Alu::writeRAM(int addr, int op) {

	tlm::tlm_generic_payload*	trans	= new tlm::tlm_generic_payload;
	tlm::tlm_command			cmd		= tlm::TLM_WRITE_COMMAND;
	sc_time						delay	= sc_time(10, SC_NS);
	int							data	= op;

	trans->set_command(cmd);
	trans->set_address(addr);
	trans->set_data_ptr(reinterpret_cast<unsigned char*>(&data));
	trans->set_data_length(4);
	trans->set_streaming_width(4);
	trans->set_byte_enable_ptr(0);
	trans->set_dmi_allowed(false);
	trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

	// Envia o dado para a RAM
	socketRAM->b_transport(*trans, delay);

	if (trans->is_response_error())
		SC_REPORT_ERROR("TLM-2", "Response error from b_transport");

	wait(delay);

}

/**
 * Executa uma escrita no topo pilha de retorno.
 * @param op
 * Dado a ser guardado.
 * @return
 * Não há retorno.
 */
void Alu::writeRS(int op) {

	tlm::tlm_generic_payload*	trans	= new tlm::tlm_generic_payload;
	tlm::tlm_command 			cmd		= tlm::TLM_WRITE_COMMAND;
	sc_time						delay	= sc_time(10, SC_NS);
	int							data	= op;

	trans->set_command(cmd);
	trans->set_address(0);
	trans->set_data_ptr(reinterpret_cast<unsigned char*>(&data));
	trans->set_data_length(4);
	trans->set_streaming_width(4);
	trans->set_byte_enable_ptr(0);
	trans->set_dmi_allowed(false);
	trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

	// Envia o dado para a pilha de retorno
	socketRS->b_transport(*trans, delay);

	if (trans->is_response_error())
		SC_REPORT_ERROR("TLM-2", "Response error from b_transport");

	wait(delay);
}

/**
 * Executa uma leitura da pilha de retorno.
 * @param addr
 * Endereço em relação ao topo da pilha a ser lido.
 * @return
 * Dado contido no endereço.
 */
int Alu::readRS(int addr) {

	tlm::tlm_generic_payload*	trans	= new tlm::tlm_generic_payload;
	tlm::tlm_command			cmd		= tlm::TLM_READ_COMMAND;
	sc_time						delay	= sc_time(10, SC_NS);
	int data;

	trans->set_command(cmd);
	trans->set_address(addr);
	trans->set_data_ptr(reinterpret_cast<unsigned char*>(&data));
	trans->set_data_length(4);
	trans->set_streaming_width(4);
	trans->set_byte_enable_ptr(0);
	trans->set_dmi_allowed(false);
	trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

	// Envia a requisição de leitura para a pilha de retorno
	socketRS->b_transport(*trans, delay);

	if (trans->is_response_error())
		SC_REPORT_ERROR("TLM-2", "Response error from b_transport");

	wait(delay);

	// Retorna o dado carregado da pilha de retorno
	return data;
}

/**
 * Executa uma escrita da pilha de dados.
 * @param op
 * Dado a ser escrito.
 * @return
 * Não há retorno.
 */
void Alu::writeDS(int op) {

	tlm::tlm_generic_payload*		trans	= new tlm::tlm_generic_payload;
	tlm::tlm_command				cmd		= tlm::TLM_WRITE_COMMAND;
	sc_time							delay	= sc_time(10, SC_NS);
	int								data	= op;

	trans->set_command(cmd);
	trans->set_address(0);
	trans->set_data_ptr(reinterpret_cast<unsigned char*>(&data));
	trans->set_data_length(4);
	trans->set_streaming_width(4);
	trans->set_byte_enable_ptr(0);
	trans->set_dmi_allowed(false);
	trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

	// Envia o dado para pilha de dados
	socketDS->b_transport(*trans, delay);

	if (trans->is_response_error())
		SC_REPORT_ERROR("TLM-2", "Response error from b_transport");

	wait(delay);
}

/**
 * Destrutor do módulo Alu.
 */
Alu::~Alu() {

}

