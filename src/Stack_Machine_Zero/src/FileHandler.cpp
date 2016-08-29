
/// FileHandler.cpp

#include "systemc"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <bitset>


using namespace sc_core;
using namespace sc_dt;
using namespace std;

/**
 * \class FileHandler
 * \brief Gerenciador de arquivos.
 *
 * Módulo responsável por carregar os arquivos de código para a memória ROM.
 *
 * \author Gerson Miguel Beckenkamp
 * \date 2015/09/05 14:16:20
 * \file FileHandler.cpp
 */
class FileHandler{
private:

	string file_name;
	ifstream program_file;

public:

	/**
	 * Construtor da classe.
	 */
	FileHandler(){}

	/**
	 * Destrutor da classe.
	*/
	~FileHandler(){}

	/**
	 * Construtor iniciando o nome do arquivo usado para carregar o código.
	 *
	 * @param name
	 * Nome do arquivo.
	 */
	FileHandler(string name){
		this->file_name=name;
	}

	/**
	 * Carrega o programa contido no arquivo especificado.
	 *
	 * @param mem
	 * Ponteiro da estrutura de memória que será prenchido.
	 */
	void LoadProgram(sc_int<32> *mem){

		string line;
		int index = 0;
		program_file.open(file_name.c_str());

		if(program_file.is_open()){

			while(!program_file.eof()){

				getline(program_file,line);
				string instruction;
				// Como as instruções são guardadas no arquivo como hexadecimal,
				// com o inicio 0x, pega somente os 8 caracteres após os dois primeiros
				instruction+=line.substr(2,10);

				unsigned ins;
				std::stringstream ss;
				ss<<hex<<instruction;
				ss>>ins;

				mem[index] = ins;

				index++;
				if(ins == 0)
					break;
			}
		}
		program_file.close();
	}
};
