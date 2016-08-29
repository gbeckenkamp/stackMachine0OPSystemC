///@file Alu.cpp
#include "Alu.h"

/**
 * Construtor do módulo.
 * @param Name
 * Nome do módulo.
 */
Alu::Alu(sc_module_name Name) :
		sc_module(Name) {
	socketInFet.register_b_transport(this, &Alu::b_transport_Fet);
}

/**
 * Inicia os buffers da pilha de dados.
 * @param buff
 * Ponteiro para o array de buffers.
 * @return
 * Não há retorno.
 */
void Alu::setBuffersDS(int buf[4]) {
	buffersDS = buf;
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
 * Inicia os buffers da pilha de retorno.
 * @param buff
 * Ponteiro para o array de buffers.
 * @return
 * Não há retorno.
 */
void Alu::setBuffersRS(int buf[4]) {
	buffersRS = buf;
}

/**
 * Inicia o endereço de base da pilha de dados na memória.
 * @param dsp
 * Ponteiro para a base da pilha de dados na memória.
 * @return
 * Não há retorno.
 */
void Alu::setDSP(int *dsp) {
	this->DSP = dsp;
}

/**
 * Inicia o endereço de base da pilha de retorno na memória.
 * @param rsp
 * Ponteiro para a base da pilha de retorno na memória.
 * @return
 * Não há retorno.
 */
void Alu::setRSP(int *rsp) {
	this->RSP = rsp;
}

/**
 * Inicia os endereços de topo das pilhas de dados e retorno na memória.
 * @param dss
 * Ponteiro para o topo da pilha de dados na memória.
 * @param rss
 * Ponteiro para o topo da pilha de retorno na memória.
 * @return
 * Não há retorno.
 */
void Alu::setSizes(int *dss, int *rss) {
	this->DSS = dss;
	this->RSS = rss;
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

	tlm::tlm_command 	cmd = trans.get_command();
	sc_dt::uint64 		adr = trans.get_address();
	unsigned char* 		ptr = trans.get_data_ptr();
	unsigned int 		len = trans.get_data_length();
	unsigned char* 		byt = trans.get_byte_enable_ptr();
	unsigned int 		wid = trans.get_streaming_width();

	if (adr >= sc_dt::uint64(SIZE) || byt != 0 || len > 4 || wid < len)
		SC_REPORT_ERROR("TLM-2",
				"Target does not support given generic payload transaction");

	// Recebe a instrução com um comando de escrita
	if (cmd == tlm::TLM_WRITE_COMMAND) {

		int i;
		memcpy(&i, ptr, len);
		instruction = i;

		// Inicia o tratamento da nova instrução
		compute();

	}

	// Termina a transação com sucesso
	trans.set_response_status(tlm::TLM_OK_RESPONSE);

}

/**
 * Interpreta a instrução atual e realiza as computações necessárias.
 * @return
 * Não há retorno.
 */
void Alu::compute() {

	if (instruction[31] == 1) {							// O bit 31 da instrução sempre indica um literal

		writeDS(2, tlm::TLM_IGNORE_COMMAND);			// Envia um comando de modulação push para a pilha de dados
		writeDS(0, (int) instruction.range(30, 0));		// Realiza a escrita na posição alocada na pilha

		*this->PC = *this->PC + 1;						// Avança PC

#ifdef DEBUG_ALU
		cout << "Lit\n";
#endif
	} else {

		int Address = 0;
		switch ((int) instruction.range(30, 28)) {
		case 1:											// Call

			writeRS(1, tlm::TLM_IGNORE_COMMAND);		// Envia um comando de modulação push para a pilha de retorno
			writeRS(0, *this->PC);						// Realiza a escrita na posição alocada na pilha

			// O bit 27 informa se o endereço de chamada está contido
			// no offset da instrução ou no topo da pilha de dados
			if (instruction[27] == 1) {

				writeDS(0, tlm::TLM_IGNORE_COMMAND);	// Modula um pop na pilha de dados
				*PC = buffersDS[0];

			} else {

				*PC = (int) instruction.range(26, 0);

			}

#ifdef DEBUG_ALU
			cout << "Call\n";
#endif
			break;

		case 2:											// Branch

			// Escolhe se a origem do endereço é o offset ou o topo da pilha de dados
			if (instruction[27] == 1){

				writeDS(0, tlm::TLM_IGNORE_COMMAND);
				*PC = buffersDS[0];

			}else
				*PC = (int) instruction.range(26, 0);

#ifdef DEBUG_ALU
			cout << "Branch\n";
#endif
			break;

		case 3:											// Branch0

			writeDS(0, tlm::TLM_IGNORE_COMMAND);		// Modula um pop na pilha de dados

			// Caso o valor no topo da pilha de dados seja 0, efetua o salto
			if (buffersDS[0] == 0) {

				// Escolhe se a origem do endereço é o offset ou o topo da pilha de dados
				if (instruction[27] == 1)
					*PC = buffersDS[0];
				else
					*PC = (int) instruction.range(26, 0);

			} else {

				*this->PC = *this->PC + 1;				// Senão avança o PC

			}

			writeDS(4, tlm::TLM_IGNORE_COMMAND);		// Atualiza os buffers

#ifdef DEBUG_ALU
			cout << "Branch0\n";
#endif
			break;

		case 5:											// Load

			Address = 0;
			bool isOffset;

			// Escolhe se a origem do endereço é o offset ou o topo da pilha de dados,
			// marcando com uma flag se o offset é usado para consumir ou não o endereço da pilha de dados
			if (instruction[25] == 1){

				Address = buffersDS[0];
				isOffset = false;

			}else{

				Address = (int) instruction.range(24, 0);
				isOffset = true;

			}

			// Escreve a leitura da memória na pilha de dados
			if ((int) instruction.range(27, 26) == 0) {

				if(isOffset)
					writeDS(1, tlm::TLM_IGNORE_COMMAND);

				writeDS(0, readRAM(Address));			// Lê a memória e aloca o valor na pilha
				writeDS(4, tlm::TLM_IGNORE_COMMAND);	// Atualiza os buffers
				*this->PC = *this->PC + 1;				// Avança o PC

			// Escreve a leitura da memória na pilha de retorno
			} else if ((int) instruction.range(27, 26) == 1) {

				if(isOffset)
					writeRS(1, tlm::TLM_IGNORE_COMMAND);

				writeRS(0, readRAM(Address));
				writeRS(4, tlm::TLM_IGNORE_COMMAND);
				*this->PC = *this->PC + 1;

			// Escreve a leitura da memória no PC
			} else if ((int) instruction.range(27, 26) == 2) {

				*PC = readRAM(Address);

			}

#ifdef DEBUG_ALU
			cout << "Load\n";
#endif
			break;

		case 6:											// Store

			Address = 0;

			// Caso o bit 25 seja 1, testa os bits 27 a 26 para determinar
			// se o endereço está no topo ou no proximo ao topo, senão pega
			// como endereço o offset da instrução
			if (instruction[25] == 1){
				if ((int) instruction.range(27, 26) == 0)
					Address = buffersDS[1];
				else
					Address = buffersDS[0];
			}else
				Address = (int) instruction.range(24, 0);


			// Escreve na memória a partir da pilha de dados
			if ((int) instruction.range(27, 26) == 0) {

				writeDS(0, tlm::TLM_IGNORE_COMMAND);	// Modula um pop na pilha de dados
				writeRAM(Address,buffersDS[0]);			// Escreve no endereço de memória o valor desejado
				writeDS(4, tlm::TLM_IGNORE_COMMAND);	// Atualiza os buffers

				*this->PC = *this->PC + 1;				// Avança o PC

			// Escreve na memória a partir da pilha de retorno
			} else if ((int) instruction.range(27, 26) == 1) {

				writeRS(0, tlm::TLM_IGNORE_COMMAND);
				writeRAM(Address,buffersRS[0]);
				writeRS(4, tlm::TLM_IGNORE_COMMAND);
				*this->PC = *this->PC + 1;

			// Escreve na memória o PC atual
			} else if ((int) instruction.range(27, 26) == 2) {

				writeRAM(Address,*PC);
				*this->PC = *this->PC + 1;

			}

#ifdef DEBUG_ALU
			cout << "Store\n";
#endif
			break;

		case 7:											// Alu
#ifdef DEBUG_ALU
			cout << "Alu\n";
#endif

			// Antes de realizar uma computação, aplica a modulação necessária nas pilhas,
			// lembrando que os dados originais ainda estão nos buffers que serão
			// atualizados no fim da execução
			switch ((int) instruction.range(19, 16)) {

			case 1:										// push na pilha de dados
				writeDS(1, tlm::TLM_IGNORE_COMMAND);
				break;

			case 2:										// pop na pilha de dados
				writeDS(0, tlm::TLM_IGNORE_COMMAND);
				break;

			case 3:										// push na pilha de retorno
				writeRS(1, tlm::TLM_IGNORE_COMMAND);
				break;

			case 4:										// pop na pilha de retorno
				writeRS(0, tlm::TLM_IGNORE_COMMAND);
				break;

			case 5:										// push nas pilhas de dados e retorno
				writeDS(1, tlm::TLM_IGNORE_COMMAND);
				writeRS(1, tlm::TLM_IGNORE_COMMAND);
				break;

			case 6:										// pop nas pilhas de dados e retorno
				writeDS(0, tlm::TLM_IGNORE_COMMAND);
				writeRS(0, tlm::TLM_IGNORE_COMMAND);
				break;

			case 7:										// push na pilha de dados e pop na pilha de retorno
				writeDS(1, tlm::TLM_IGNORE_COMMAND);
				writeRS(0, tlm::TLM_IGNORE_COMMAND);
				break;

			case 8:										// pop na pilha de dados e push na pilha de retorno
				writeDS(0, tlm::TLM_IGNORE_COMMAND);
				writeRS(1, tlm::TLM_IGNORE_COMMAND);
				break;
			}

			int equal = 0;

			// Testa o opcode da Alu para determinar a computação a ser feita
			switch ((int) instruction.range(15, 11)) {

			case 0:										// Nada

				*this->PC = *this->PC + 1;
				writeDS(4, tlm::TLM_IGNORE_COMMAND);

				break;

			case 1:										// Add

				// Testa qual pilha é a origem
				if ((int) instruction.range(27, 24) == 1) {

					// Testa qual pilha é o destino
					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, buffersDS[0] + buffersDS[1]);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, buffersDS[0] + buffersDS[1]);
						*this->PC = *this->PC + 1;

					}
				} else if ((int) instruction.range(27, 24) == 5) {

					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, buffersRS[0] + buffersRS[1]);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, buffersRS[0] + buffersRS[1]);
						*this->PC = *this->PC + 1;

					}
				}
				break;

			case 2:										// Sub

				if ((int) instruction.range(27, 24) == 1) {

					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, buffersDS[0] - buffersDS[1]);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, buffersDS[0] - buffersDS[1]);
						*this->PC = *this->PC + 1;

					}
				} else if ((int) instruction.range(27, 24) == 5) {

					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, buffersRS[0] - buffersRS[1]);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, buffersRS[0] - buffersRS[1]);
						*this->PC = *this->PC + 1;

					}
				}
				break;

			case 3:										// Mult
				if ((int) instruction.range(27, 24) == 1) {

					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, buffersDS[0] * buffersDS[1]);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, buffersDS[0] * buffersDS[1]);
						*this->PC = *this->PC + 1;

					}
				} else if ((int) instruction.range(27, 24) == 5) {

					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, buffersRS[0] * buffersRS[1]);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, buffersRS[0] * buffersRS[1]);
						*this->PC = *this->PC + 1;

					}
				}
				break;

			case 4:										// Div
				if ((int) instruction.range(27, 24) == 1) {

					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, buffersDS[0] / buffersDS[1]);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, buffersDS[0] / buffersDS[1]);
						*this->PC = *this->PC + 1;

					}
				} else if ((int) instruction.range(27, 24) == 5) {

					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, buffersRS[0] / buffersRS[1]);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, buffersRS[0] / buffersRS[1]);
						*this->PC = *this->PC + 1;

					}
				}
				break;

			case 5:										// And
				if ((int) instruction.range(27, 24) == 1) {

					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, buffersDS[0] & buffersDS[1]);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, buffersDS[0] & buffersDS[1]);
						*this->PC = *this->PC + 1;

					}
				} else if ((int) instruction.range(27, 24) == 5) {

					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, buffersRS[0] & buffersRS[1]);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, buffersRS[0] & buffersRS[1]);
						*this->PC = *this->PC + 1;

					}
				} else if ((int) instruction.range(27, 24) == 10) {

					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, buffersDS[0] & 0);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, buffersDS[0] & 0);
						*this->PC = *this->PC + 1;

					}
				}
				break;

			case 6:										// Or
				if ((int) instruction.range(27, 24) == 1) {

					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, buffersDS[0] | buffersDS[1]);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, buffersDS[0] | buffersDS[1]);
						*this->PC = *this->PC + 1;

					}

				} else if ((int) instruction.range(27, 24) == 5) {

					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, buffersRS[0] | buffersRS[1]);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, buffersRS[0] | buffersRS[1]);
						*this->PC = *this->PC + 1;

					}
				} else if ((int) instruction.range(27, 24) == 10) {

					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, buffersDS[0] | 0);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, buffersDS[0] | 0);
						*this->PC = *this->PC + 1;

					}
				}

				break;

			case 7:										// Xor
				if ((int) instruction.range(27, 24) == 1) {

					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, buffersDS[0] ^ buffersDS[1]);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, buffersDS[0] ^ buffersDS[1]);
						*this->PC = *this->PC + 1;

					}
				} else if ((int) instruction.range(27, 24) == 5) {

					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, buffersRS[0] ^ buffersRS[1]);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, buffersRS[0] ^ buffersRS[1]);
						*this->PC = *this->PC + 1;

					}
				} else if ((int) instruction.range(27, 24) == 10) {

					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, buffersDS[0] ^ 0);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, buffersDS[0] ^ 0);
						*this->PC = *this->PC + 1;

					}
				}

				break;

			case 8:										// Xnor
				if ((int) instruction.range(27, 24) == 1) {

					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, ~(buffersDS[0] ^ buffersDS[1]));
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, ~(buffersDS[0] ^ buffersDS[1]));
						*this->PC = *this->PC + 1;

					}
				} else if ((int) instruction.range(27, 24) == 5) {

					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, ~(buffersRS[0] ^ buffersRS[1]));
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, ~(buffersRS[0] ^ buffersRS[1]));
						*this->PC = *this->PC + 1;

					}
				} else if ((int) instruction.range(27, 24) == 10) {

					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, ~(buffersDS[0] ^ 0));
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, ~(buffersDS[0] ^ 0));
						*this->PC = *this->PC + 1;

					}
				}
				break;

			case 9:										// Equal
				equal = 0;
				if ((int) instruction.range(27, 24) == 1) {

					if (buffersDS[0] == buffersDS[1])
						equal = 1;
					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, equal);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, equal);
						*this->PC = *this->PC + 1;

					}
				} else if ((int) instruction.range(27, 24) == 5) {

					if (buffersRS[0] == buffersRS[1])
						equal = 1;
					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, equal);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, equal);
						*this->PC = *this->PC + 1;

					}
				} else if ((int) instruction.range(27, 24) == 10) {

					if (buffersDS[0] == 0)
						equal = 1;
					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, equal);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, equal);
						*this->PC = *this->PC + 1;

					}
				}
				break;

			case 10:									// Different

				equal = 0;

				if ((int) instruction.range(27, 24) == 1) {

					if (buffersDS[0] != buffersDS[1])
						equal = 1;
					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, equal);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, equal);
						*this->PC = *this->PC + 1;

					}
				} else if ((int) instruction.range(27, 24) == 5) {

					if (buffersRS[0] != buffersRS[1])
						equal = 1;
					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, equal);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, equal);
						*this->PC = *this->PC + 1;

					}
				} else if ((int) instruction.range(27, 24) == 10) {

					if (buffersRS[0] != 0)
						equal = 1;
					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, equal);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, equal);
						*this->PC = *this->PC + 1;

					}
				}
				break;

			case 11:									// More or equal

				equal = 0;
				if ((int) instruction.range(27, 24) == 1) {

					if (buffersDS[0] >= buffersDS[1])
						equal = 1;
					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, equal);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, equal);
						*this->PC = *this->PC + 1;

					}
				} else if ((int) instruction.range(27, 24) == 5) {

					if (buffersRS[0] >= buffersRS[1])
						equal = 1;
					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, equal);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, equal);
						*this->PC = *this->PC + 1;

					}
				} else if ((int) instruction.range(27, 24) == 10) {

					if (buffersRS[0] >= 0)
						equal = 1;
					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, equal);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, equal);
						*this->PC = *this->PC + 1;

					}
				}
				break;

			case 12:									// Minor or equal

				equal = 0;
				if ((int) instruction.range(27, 24) == 1) {

					if (buffersDS[0] <= buffersDS[1])
						equal = 1;
					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, equal);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, equal);
						*this->PC = *this->PC + 1;

					}
				} else if ((int) instruction.range(27, 24) == 5) {

					if (buffersRS[0] <= buffersRS[1])
						equal = 1;
					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, equal);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, equal);
						*this->PC = *this->PC + 1;

					}
				} else if ((int) instruction.range(27, 24) == 10) {

					if (buffersRS[0] <= 0)
						equal = 1;
					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, equal);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, equal);
						*this->PC = *this->PC + 1;

					}
				}
				break;

			case 13:									// More

				equal = 0;
				if ((int) instruction.range(27, 24) == 1) {

					if (buffersDS[0] > buffersDS[1])
						equal = 1;
					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, equal);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, equal);
						*this->PC = *this->PC + 1;

					}
				} else if ((int) instruction.range(27, 24) == 5) {

					if (buffersRS[0] > buffersRS[1])
						equal = 1;
					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, equal);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, equal);
						*this->PC = *this->PC + 1;

					}
				} else if ((int) instruction.range(27, 24) == 10) {

					if (buffersRS[0] > 0)
						equal = 1;
					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, equal);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, equal);
						*this->PC = *this->PC + 1;

					}
				}
				break;

			case 14:									// Minor

				equal = 0;
				if ((int) instruction.range(27, 24) == 1) {

					if (buffersDS[0] < buffersDS[1])
						equal = 1;
					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, equal);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, equal);
						*this->PC = *this->PC + 1;

					}
				} else if ((int) instruction.range(27, 24) == 5) {

					if (buffersRS[0] < buffersRS[1])
						equal = 1;
					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, equal);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, equal);
						*this->PC = *this->PC + 1;

					}
				} else if ((int) instruction.range(27, 24) == 10) {

					if (buffersRS[0] < 0)
						equal = 1;
					if ((int) instruction.range(23, 20) == 1) {

						writeDS(0, equal);
						*this->PC = *this->PC + 1;

					} else if ((int) instruction.range(23, 20) == 5) {

						writeRS(0, equal);
						*this->PC = *this->PC + 1;

					}
				}
				break;

			case 15:									// Not

				storeSource((int) instruction.range(23, 20),
						~pickSource((int) instruction.range(27, 24)));
				break;

			case 16:									// Shift left

				storeSource((int) instruction.range(23, 20),
						pickSource((int) instruction.range(27, 24))
								<< ((int) instruction.range(9, 0)));
				break;

			case 17:									// Shift right

				storeSource((int) instruction.range(23, 20),
						pickSource((int) instruction.range(27, 24))
								>> ((int) instruction.range(9, 0)));
				break;

			case 18:									// Add offset

				storeSource((int) instruction.range(23, 20),
						pickSource((int) instruction.range(27, 24))
								+ ((int) instruction.range(9, 0)));
				break;

			case 19:									// Sub offset

				storeSource((int) instruction.range(23, 20),
						pickSource((int) instruction.range(27, 24))
								- ((int) instruction.range(9, 0)));
				break;

			case 20:									// Mult offset

				storeSource((int) instruction.range(23, 20),
						pickSource((int) instruction.range(27, 24))
								* ((int) instruction.range(9, 0)));
				break;

			case 21:									// Move

				storeSource((int) instruction.range(23, 20),
						pickSource((int) instruction.range(27, 24)));
				break;
			}

			// Caso a instrução tenha retorno de função, passa o topo da pilha de retorno
			// para o PC, efetua um pop na mesma e avança o novo PC.
			if(instruction[10] == 1){

				*this->PC = buffersRS[0];
				writeRS(3,tlm::TLM_IGNORE_COMMAND);
				*this->PC = *this->PC+1;

			}

			break;
		}
	}
}

/**
 * Executa uma escrita da pilha de dados.
 * @param adr
 * Posição da pilha a ser escrita.
 * @param op
 * Dado a ser escrito.
 * @return
 * Não há retorno.
 */
void Alu::writeDS(int adr, int op) {

	tlm::tlm_generic_payload* 	trans 	= new tlm::tlm_generic_payload;
	tlm::tlm_command 			cmd 	= tlm::TLM_WRITE_COMMAND;
	sc_time 					delay 	= sc_time(10, SC_NS);
	int 						data 	= op;

	trans->set_command(cmd);
	trans->set_address(adr);
	trans->set_data_ptr(reinterpret_cast<unsigned char*>(&data));
	trans->set_data_length(4);
	trans->set_streaming_width(4);
	trans->set_byte_enable_ptr(0);
	trans->set_dmi_allowed(false);
	trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

	// Envia a transação para a pilha de dados
	socketDS->b_transport(*trans, delay);

	if (trans->is_response_error())
		SC_REPORT_ERROR("TLM-2", "Response error from b_transport");

	wait(delay);
}

/**
 * Executa uma escrita no topo pilha de retorno.
 * @param adr
 * Posição da pilha a ser escrita.
 * @param op
 * Dado a ser guardado.
 * @return
 * Não há retorno.
 */
void Alu::writeRS(int adr, int op) {

	tlm::tlm_generic_payload* 	trans	 = new tlm::tlm_generic_payload;
	tlm::tlm_command 			cmd		 = tlm::TLM_WRITE_COMMAND;
	sc_time 					delay	 = sc_time(10, SC_NS);
	int 						data	 = op;

	trans->set_command(cmd);
	trans->set_address(adr);
	trans->set_data_ptr(reinterpret_cast<unsigned char*>(&data));
	trans->set_data_length(4);
	trans->set_streaming_width(4);
	trans->set_byte_enable_ptr(0);
	trans->set_dmi_allowed(false);
	trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

	// Envia a tranação para a pilha de retorno
	socketRS->b_transport(*trans, delay);

	if (trans->is_response_error())
		SC_REPORT_ERROR("TLM-2", "Response error from b_transport");

	wait(delay);
}

/**
 * Executa uma modulação no topo da pilha de retorno.
 * @param adr
 * Tipo de modulação.
 * @param cmd
 * Comando identificando uma modulação. Padrão = TLM_IGNORE_COMMAND.
 * @return
 * Não há retorno.
 */
void Alu::writeRS(int adr, tlm::tlm_command cmd) {

	tlm::tlm_generic_payload* 	trans = new tlm::tlm_generic_payload;
	sc_time 					delay = sc_time(10, SC_NS);
	int 						data;

	trans->set_command(cmd);
	trans->set_address(adr);
	trans->set_data_ptr(reinterpret_cast<unsigned char*>(&data));
	trans->set_data_length(4);
	trans->set_streaming_width(4);
	trans->set_byte_enable_ptr(0);
	trans->set_dmi_allowed(false);
	trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

	// Envia a transação para o pilha de retorno
	socketRS->b_transport(*trans, delay);

	if (trans->is_response_error())
		SC_REPORT_ERROR("TLM-2", "Response error from b_transport");

	wait(delay);
}

/**
 * Executa uma escrita da pilha de dados.
 * @param adr
 * Tipo de modulação.
 * @param cmd
 * Comando identificando uma modulação. Padrão = TLM_IGNORE_COMMAND.
 * @return
 * Não há retorno.
 */
void Alu::writeDS(int adr, tlm::tlm_command cmd) {

	tlm::tlm_generic_payload* 	trans = new tlm::tlm_generic_payload;
	sc_time 					delay = sc_time(10, SC_NS);
	int 						data;

	trans->set_command(cmd);
	trans->set_address(adr);
	trans->set_data_ptr(reinterpret_cast<unsigned char*>(&data));
	trans->set_data_length(4);
	trans->set_streaming_width(4);
	trans->set_byte_enable_ptr(0);
	trans->set_dmi_allowed(false);
	trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

	// Envia a transação para a pilha de dados
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

	tlm::tlm_generic_payload* trans = new tlm::tlm_generic_payload;
	tlm::tlm_command cmd = tlm::TLM_WRITE_COMMAND;
	sc_time delay = sc_time(10, SC_NS);
	int data = op;

	trans->set_command(cmd);
	trans->set_address(addr);
	trans->set_data_ptr(reinterpret_cast<unsigned char*>(&data));
	trans->set_data_length(4);
	trans->set_streaming_width(4);
	trans->set_byte_enable_ptr(0);
	trans->set_dmi_allowed(false);
	trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

	// Envia a transação para a memória
	socketRAM->b_transport(*trans, delay);

	if (trans->is_response_error())
		SC_REPORT_ERROR("TLM-2", "Response error from b_transport");

	wait(delay);
}

/**
 * Executa a leitura da memória RAM.
 * @param addr
 * Endereço de memória a ser lido.
 * @return
 * Valor lido no endereço especificado.
 */
int Alu::readRAM(int addr) {

	tlm::tlm_generic_payload* 	trans 	= new tlm::tlm_generic_payload;
	tlm::tlm_command 			cmd 	= tlm::TLM_READ_COMMAND;
	sc_time 					delay 	= sc_time(10, SC_NS);
	int 						data;

	trans->set_command(cmd);
	trans->set_address(addr);
	trans->set_data_ptr(reinterpret_cast<unsigned char*>(&data));
	trans->set_data_length(4);
	trans->set_streaming_width(4);
	trans->set_byte_enable_ptr(0);
	trans->set_dmi_allowed(false);
	trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

	// Envia a transação para a memória
	socketRAM->b_transport(*trans, delay);

	if (trans->is_response_error())
		SC_REPORT_ERROR("TLM-2", "Response error from b_transport");

	wait(delay);

	return data;
}

/**
 * Retorna o valor contido na origem especificada pelo campo source da instrução.
 * @param op
 * Campo source da instrução.
 * @return
 * Valor contido na origem especificada.
 */
int Alu::pickSource(int op) {

	switch (op) {

	case 0:					// Nothing

		return 0;
		break;

	case 1: 				// Tos

		return buffersDS[0];
		break;

	case 2:					// Ntos

		return buffersDS[1];
		break;

	case 3: 				// 3os

		return buffersDS[2];
		break;

	case 4: 				// 4os

		return buffersDS[3];
		break;

	case 5: 				// Tors

		return buffersRS[0];
		break;

	case 6: 				// Ntors

		return buffersRS[1];
		break;

	case 7: 				// 3ors

		return buffersRS[2];
		break;

	case 8: 				// 4ors

		return buffersRS[3];
		break;

	case 9: 				// PC

		return *PC;
		break;

	case 10: 				// Zero

		return 0;
		break;

	case 11: 				// DS size

		return *DSS;
		break;

	case 12: 				// Rs size

		return *RSS;
		break;

	case 13: 				// DSP

		return *DSP;
		break;

	case 14: 				// RSP

		return *RSP;
		break;

	default :
		return 0;

	}
}

/**
 * Guarda o dado computado no destino especificado no campo destination da instrução.
 * @param op
 * Campo destination da instrução.
 * @param data
 * Dado a ser armazenado no destino.
 * @return
 * Valor contido na origem especificada.
 */
void Alu::storeSource(int op, int data) {

	switch (op) {

	case 0:					// Nothing

		*this->PC = *this->PC + 1;
		break;

	case 1: 				// Tos

		writeDS(0, data);
		*this->PC = *this->PC + 1;
		break;

	case 2: 				// Ntos

		writeDS(1, data);
		*this->PC = *this->PC + 1;
		break;

	case 3: 				// 3os

		writeDS(2, data);
		*this->PC = *this->PC + 1;
		break;

	case 4: 				// 4os

		writeDS(3, data);
		*this->PC = *this->PC + 1;
		break;

	case 5: 				// Tors

		writeRS(0, data);
		*this->PC = *this->PC + 1;
		break;

	case 6: 				// Ntors

		writeRS(1, data);
		*this->PC = *this->PC + 1;
		break;

	case 7: 				// 3ors

		writeRS(2, data);
		*this->PC = *this->PC + 1;
		break;

	case 8: 				// 4ors

		writeRS(3, data);
		*this->PC = *this->PC + 1;
		break;

	case 9: 				//PC

		*this->PC = data;
		break;

	case 10: 				// Zero

		*this->PC = *this->PC + 1;
		break;

	case 11: 				// DS size

		*this->PC = *this->PC + 1;
		break;

	case 12: 				//Rs size

		*this->PC = *this->PC + 1;
		break;

	case 13:			 	// DSP

		*this->DSP = data;
		*this->PC = *this->PC + 1;
		break;

	case 14: 				// RSP

		*this->RSP = data;
		*this->PC = *this->PC + 1;

		break;
	}
}

/**
 * Destrutor do módulo
 */
Alu::~Alu(){

}
