/*
 * Dual_OP.cpp
 *
 *  Created on: Aug 20, 2015
 *      Author: nuts
 */

#include "Dual_OP.h"

/**
 * Construtor.
 */
Assembler_Dual::Assembler_Dual(char* entry[]) {

	this->file_name 	= entry[1];
	this->targetPath 	= entry[2];
	head = NULL;

}

/**
 * Destrutor
 */
Assembler_Dual::~Assembler_Dual(){

	Instruction *i;
	i = head->next;
	if(i==NULL)
		return;
	else
		free(head);

	do{

		head = i->next;
		free(i);
		i = head;

	}while(i->next != NULL);

}

/**
 * Carrega o arquivo de programa e monta um vetor dos dados contidos em cada instrução.
 */
void Assembler_Dual::Load_Program() {
	string line;
	program_file.open(file_name.c_str());

	Instruction *last;
	if (program_file.is_open()) {

		while (!program_file.eof()) {		//read until the end of the file
			getline(program_file, line);

			string sub[4];
			Instruction* next_ins = (Instruction*) malloc(sizeof(Instruction));	//allocate the next instruction
			next_ins->next = NULL;

			next_ins->isReturn = containWord(line, ";");

			int indx = 0;						//count the number of operands
			int next = 0;				//keep the address of found whitespaces
			while (line.find(' ') != string::npos && indx < 4) {//pick up all the whitespaces

				next = line.find(' ');

				if (line.substr(0, next).compare("##") == 0) {
					line = "";
					break;
				}

				if (line.substr(0, next).compare("") != 0) {//don't pick empty strings
					sub[indx] = line.substr(0, next);//pick the string after the whitespace
					indx++;
				}
				if ((next + 1) < line.length())	//if its not the end of the string, chop off the characters after the whitespace and the whitespace
					line = line.substr(next + 1, line.length());
				else
					break;

			}

			if (line.length() > 0 && indx < 4) {//in cases where the string have only one space, pick up the rest of the
				sub[indx] = line.substr(0, line.length());
				indx++;
			}

			next_ins->type = getType(sub[0]);
			next_ins->cnt = indx;
			if (next_ins->type == 0)
				next_ins->op1 = getInt(sub[0]);
			else if (indx == 2)
				next_ins->op1 = getInt(sub[1]);
			else if (indx == 3) {
				next_ins->op1 = getInt(sub[1]);
				next_ins->op2 = getInt(sub[2]);
			} else if (indx == 4) {
				next_ins->op1 = getInt(sub[1]);
				next_ins->op2 = getInt(sub[2]);
				next_ins->op3 = getInt(sub[3]);
			}

			if (head == NULL) {
				head = next_ins;
			} else {
				for (last = head; last->next != NULL; last = last->next)
					;
				last->next = next_ins;
			}
		}
	}
	program_file.close();
}

/**
 * Retorna o valor inteiro da string especificada.
 * @param s
 * String contendo o valor inteiro
 * @return
 * Valor inteiro da string
 */
int Assembler_Dual::getInt (string s) {
	unsigned ins;
	std::stringstream ss;
	ss << s;
	ss >> ins;
	return ins;
}


/**
 * Retorna o identificador de macro do comando especificado.
 * @param s
 * String contendo o comando da instrução lida
 * @return
 * Identificador da macro.
 */
int Assembler_Dual::getType (string s) {
	if (containWord(s, "add")) {
		return 1;
	} else if (containWord(s, "sub")) {
		return 2;
	} else if (containWord(s, "calleq")) {
		return 29;
	} else if (containWord(s, "mlt")) {
		return 3;
	} else if (containWord(s, "div")) {
		return 4;
	} else if (containWord(s, "and")) {
		return 5;
	} else if (containWord(s, "xor")) {
		return 7;
	} else if (containWord(s, "xnor")) {
		return 8;
	} else if (containWord(s, "equal") || containWord(s, "==")) {
		return 9;
	} else if (containWord(s, "dif") || containWord(s, "!=")) {
		return 10;
	} else if (containWord(s, "bme")) {
		return 24;
	} else if (containWord(s, "me") || containWord(s, ">=")) {
		return 11;
	} else if (containWord(s, "le") || containWord(s, "<=")) {
		return 12;
	} else if (containWord(s, "more") || containWord(s, ">")) {
		return 13;
	} else if (containWord(s, "less") || containWord(s, "<")) {
		return 14;
	} else if (containWord(s, "sr") || containWord(s, ">>")) {
		return 15;
	} else if (containWord(s, "not")) {
		return 16;
	} else if (containWord(s, "sl") || containWord(s, "<<")) {
		return 17;
	} else if (containWord(s, "move")) {
		return 18;
	} else if (containWord(s, "load")) {
		return 19;
	} else if (containWord(s, "store")) {
		return 20;
	} else if (containWord(s, "beq")) {
		return 22;
	} else if (containWord(s, "bdif")) {
		return 23;
	} else if (containWord(s, "bm")) {
		return 25;
	} else if (containWord(s, "branch")) {
		return 26;
	} else if (containWord(s, "nop")) {
		return 27;
	} else if (containWord(s, "return")) {
		return 28;
	} else if (containWord(s, "or")) {
		return 6;
	} else if (containWord(s, "calldif")) {
		return 30;
	} else if (containWord(s, "callme")) {
		return 31;
	} else if (containWord(s, "callm")) {
		return 32;
	} else if (containWord(s, "call")) {
		return 21;
	} else if (containWord(s, "++")) {
		return 33;
	} else if (containWord(s, "--")) {
		return 34;
	} else if (containWord(s, "stro")) {
		return 35;
	} else {
		return 0;
	}
}

/**
 * Interpreta os dados retirados do programa e escreve a instrução de máquina no arquivo binário.
 */
void Assembler_Dual::writeProgram() {

	Instruction *i = head;
	ofstream target(this->targetPath);

	if (target.is_open()) {

		do {

			cout << "type " << i->type << "\n";

			if (i->type == 0) {
				bitset<32> instruc(i->op1);
				instruc.set(31, 1);

				target << writeIns(instruc);

			} else if (i->type == 1) {		//add

				bitset<32> instruc(0);
				instruc.set(30, 1);
				instruc.set(29, 1);

				if (i->isReturn) {
					instruc.set(14, 1);
					instruc.set(12, 1);
				}

				putOperand(i->op1, 1, instruc);
				putOperand(i->op2, 0, instruc);

				target << writeIns(instruc);

			} else if (i->type == 2) {			//sub

				bitset<32> instruc(0);
				instruc.set(30, 1);
				instruc.set(29, 1);

				instruc.set(8, 1);
				if (i->isReturn) {
					instruc.set(14, 1);
					instruc.set(12, 1);
				}

				putOperand(i->op1, 1, instruc);
				putOperand(i->op2, 0, instruc);

				target << writeIns(instruc);

			} else if (i->type == 3) {		//mlt

				bitset<32> instruc(0);
				instruc.set(30, 1);
				instruc.set(29, 1);

				instruc.set(8, 1);
				instruc.set(10, 1);
				instruc.set(11, 1);
				if (i->isReturn) {
					instruc.set(14, 1);
					instruc.set(12, 1);
				}

				putOperand(i->op1, 1, instruc);
				putOperand(i->op2, 0, instruc);

				target << writeIns(instruc);

			} else if (i->type == 4) {		//div

				bitset<32> instruc(0);
				instruc.set(30, 1);
				instruc.set(29, 1);

				instruc.set(9, 1);
				instruc.set(10, 1);

				if (i->isReturn) {
					instruc.set(14, 1);
					instruc.set(12, 1);
				}

				putOperand(i->op1, 1, instruc);
				putOperand(i->op2, 0, instruc);

				target << writeIns(instruc);

			} else if (i->type == 6) {		//or

				bitset<32> instruc(0);
				instruc.set(30, 1);
				instruc.set(29, 1);

				instruc.set(8, 1);
				instruc.set(9, 1);

				if (i->isReturn) {
					instruc.set(14, 1);
					instruc.set(12, 1);
				}

				putOperand(i->op1, 1, instruc);
				putOperand(i->op2, 0, instruc);

				target << writeIns(instruc);

			} else if (i->type == 7) {		//xor

				bitset<32> instruc(0);
				instruc.set(30, 1);
				instruc.set(29, 1);

				instruc.set(10, 1);

				if (i->isReturn) {
					instruc.set(14, 1);
					instruc.set(12, 1);
				}

				putOperand(i->op1, 1, instruc);
				putOperand(i->op2, 0, instruc);

				target << writeIns(instruc);

			} else if (i->type == 8) {		//xnor

				bitset<32> instruc(0);
				instruc.set(30, 1);
				instruc.set(29, 1);

				instruc.set(8, 1);
				instruc.set(10, 1);

				if (i->isReturn) {
					instruc.set(14, 1);
					instruc.set(12, 1);
				}

				putOperand(i->op1, 1, instruc);
				putOperand(i->op2, 0, instruc);

				target << writeIns(instruc);

			} else if (i->type == 9) {		//==

				bitset<32> instruc(0);
				instruc.set(30, 1);
				instruc.set(29, 1);

				instruc.set(11, 1);

				if (i->isReturn) {
					instruc.set(14, 1);
					instruc.set(12, 1);
				}

				putOperand(i->op1, 1, instruc);
				putOperand(i->op2, 0, instruc);

				target << writeIns(instruc);

			} else if (i->type == 10) {		//!=

				bitset<32> instruc(0);
				instruc.set(30, 1);
				instruc.set(29, 1);

				instruc.set(11, 1);
				instruc.set(8, 1);

				if (i->isReturn) {
					instruc.set(14, 1);
					instruc.set(12, 1);
				}

				putOperand(i->op1, 1, instruc);
				putOperand(i->op2, 0, instruc);

				target << writeIns(instruc);

			} else if (i->type == 11) {		//>=

				bitset<32> instruc(0);
				instruc.set(30, 1);
				instruc.set(29, 1);

				instruc.set(11, 1);
				instruc.set(9, 1);

				if (i->isReturn) {
					instruc.set(14, 1);
					instruc.set(12, 1);
				}

				putOperand(i->op1, 1, instruc);
				putOperand(i->op2, 0, instruc);

				target << writeIns(instruc);

			} else if (i->type == 12) {		//<=

				bitset<32> instruc(0);
				instruc.set(30, 1);
				instruc.set(29, 1);

				instruc.set(11, 1);
				instruc.set(9, 1);
				instruc.set(8, 1);

				if (i->isReturn) {
					instruc.set(14, 1);
					instruc.set(12, 1);
				}

				putOperand(i->op1, 1, instruc);
				putOperand(i->op2, 0, instruc);

				target << writeIns(instruc);

			} else if (i->type == 13) {		//more

				bitset<32> instruc(0);
				instruc.set(30, 1);
				instruc.set(29, 1);

				instruc.set(10, 1);
				instruc.set(9, 1);
				instruc.set(8, 1);

				if (i->isReturn) {
					instruc.set(14, 1);
					instruc.set(12, 1);
				}

				putOperand(i->op1, 1, instruc);
				putOperand(i->op2, 0, instruc);

				target << writeIns(instruc);

			} else if (i->type == 14) {		//less

				bitset<32> instruc(0);
				instruc.set(30, 1);
				instruc.set(29, 1);

				instruc.set(11, 1);
				instruc.set(10, 1);

				if (i->isReturn) {
					instruc.set(14, 1);
					instruc.set(12, 1);
				}

				putOperand(i->op1, 1, instruc);
				putOperand(i->op2, 0, instruc);

				target << writeIns(instruc);

			} else if (i->type == 15) {		//shifl right

				bitset<32> instruc(i->op2);
				instruc <<= 15;
				instruc.set(30, 1);
				instruc.set(29, 1);

				instruc.set(0, 1);
				instruc.set(9, 1);
				instruc.set(28, 1);

				if (i->isReturn) {
					instruc.set(14, 1);
					instruc.set(12, 1);
				}

				putOperand(i->op1, 1, instruc);

				target << writeIns(instruc);

			} else if (i->type == 16) {		//not

				bitset<32> instruc(0);
				instruc.set(30, 1);
				instruc.set(29, 1);

				instruc.set(0, 1);
				instruc.set(28, 1);

				if (i->isReturn) {
					instruc.set(14, 1);
					instruc.set(12, 1);
				}

				putOperand(i->op1, 1, instruc);

				target << writeIns(instruc);

			} else if (i->type == 17) {		//shift left

				bitset<32> instruc(i->op2);
				instruc <<= 15;
				instruc.set(30, 1);
				instruc.set(29, 1);

				instruc.set(9, 1);
				instruc.set(28, 1);

				if (i->isReturn) {
					instruc.set(14, 1);
					instruc.set(12, 1);
				}

				putOperand(i->op1, 1, instruc);

				target << writeIns(instruc);

			} else if (i->type == 19) {	//load
				bitset<32> instruc(0);

				instruc.set(30, 1);
				instruc.set(29, 0);
				instruc.set(27, 1);
				putOperand(i->op1, 1, instruc);

				target << writeIns(instruc);

			} else if (i->type == 20) {	//store

				bitset<32> instruc(0);

				instruc.set(30, 1);
				instruc.set(29, 0);
				instruc.set(28, 1);
				instruc.set(27, 1);

				putOperand(i->op1, 1, instruc);
				putOperand(i->op2, 0, instruc);

				target << writeIns(instruc);

			} else if (i->type == 21) {	//call

				bitset<32> instruc(i->op1);
				instruc <<= 11;
				instruc.set(31, 0);
				instruc.set(30, 0);
				instruc.set(29, 1);
				instruc.set(28, 0);
				instruc.set(8, 1);

				target << writeIns(instruc);

			} else if (i->type == 22) {	//beq

				bitset<32> instruc(i->op3);
				instruc <<= 11;
				instruc.set(31, 0);
				instruc.set(30, 0);
				instruc.set(29, 1);
				instruc.set(28, 0);
				instruc.set(10, 0);
				instruc.set(9, 0);
				instruc.set(8, 0);

				putOperand(i->op1, 1, instruc);
				putOperand(i->op2, 0, instruc);

				target << writeIns(instruc);

			} else if (i->type == 23) {	//bdif

				bitset<32> instruc(i->op3);
				instruc <<= 11;
				instruc.set(31, 0);
				instruc.set(30, 0);
				instruc.set(29, 1);
				instruc.set(28, 0);
				instruc.set(10, 0);
				instruc.set(9, 1);
				instruc.set(8, 0);

				putOperand(i->op1, 1, instruc);
				putOperand(i->op2, 0, instruc);

				target << writeIns(instruc);

			} else if (i->type == 24) {	//bme

				bitset<32> instruc(i->op3);
				instruc <<= 11;
				instruc.set(31, 0);
				instruc.set(30, 0);
				instruc.set(29, 1);
				instruc.set(28, 0);
				instruc.set(10, 1);
				instruc.set(9, 1);
				instruc.set(8, 0);

				putOperand(i->op1, 1, instruc);
				putOperand(i->op2, 0, instruc);

				target << writeIns(instruc);

			} else if (i->type == 25) {	//be

				bitset<32> instruc(i->op3);
				instruc <<= 11;
				instruc.set(31, 0);
				instruc.set(30, 0);
				instruc.set(29, 1);
				instruc.set(28, 0);
				instruc.set(10, 1);
				instruc.set(8, 0);

				putOperand(i->op1, 1, instruc);
				putOperand(i->op2, 0, instruc);

				target << writeIns(instruc);

			} else if (i->type == 26) {	//branch

				bitset<32> instruc(i->op1);
				instruc <<= 11;
				instruc.set(31, 0);
				instruc.set(30, 0);
				instruc.set(29, 1);
				instruc.set(28, 1);
				instruc.set(8, 0);

				target << writeIns(instruc);

			} else if (i->type == 27) {	//nop
				bitset<32> instruc(0);
				target << writeIns(instruc);

			} else if (i->type == 28) {	//return
				bitset<32> instruc(0);
				instruc.set(30, 1);
				instruc.set(29, 1);
				instruc.set(28, 1);
				instruc.set(14, 1);
				instruc.set(12, 1);
				target << writeIns(instruc);

			} else if (i->type == 29) {	//calleq

				bitset<32> instruc(i->op3);
				instruc <<= 11;
				instruc.set(31, 0);
				instruc.set(30, 0);
				instruc.set(29, 1);
				instruc.set(28, 0);
				instruc.set(8, 1);

				putOperand(i->op1, 1, instruc);
				putOperand(i->op2, 0, instruc);

				target << writeIns(instruc);

			} else if (i->type == 30) { //calldif

				bitset<32> instruc(i->op3);
				instruc <<= 11;
				instruc.set(31, 0);
				instruc.set(30, 0);
				instruc.set(29, 1);
				instruc.set(28, 0);
				instruc.set(9, 1);
				instruc.set(8, 1);

				putOperand(i->op1, 1, instruc);
				putOperand(i->op2, 0, instruc);

				target << writeIns(instruc);

			} else if (i->type == 31) { //callme

				bitset<32> instruc(i->op3);
				instruc <<= 11;
				instruc.set(31, 0);
				instruc.set(30, 0);
				instruc.set(29, 1);
				instruc.set(28, 0);
				instruc.set(10, 1);
				instruc.set(9, 1);
				instruc.set(8, 1);

				putOperand(i->op1, 1, instruc);
				putOperand(i->op2, 0, instruc);

				target << writeIns(instruc);

			} else if (i->type == 32) { //callm

				bitset<32> instruc(i->op3);
				instruc <<= 11;
				instruc.set(31, 0);
				instruc.set(30, 0);
				instruc.set(29, 1);
				instruc.set(28, 0);
				instruc.set(10, 1);
				instruc.set(8, 1);

				putOperand(i->op1, 1, instruc);
				putOperand(i->op2, 0, instruc);

				target << writeIns(instruc);

			} else if (i->type == 33) { //++

				bitset<32> instruc(i->op2);
				instruc <<= 15;
				instruc.set(31, 0);
				instruc.set(30, 1);
				instruc.set(29, 1);
				instruc.set(28, 1);
				instruc.set(10, 1);
				putOperand(i->op1, 1, instruc);

				target << writeIns(instruc);

			} else if (i->type == 34) { //--

				bitset<32> instruc(i->op2);
				instruc <<= 15;
				instruc.set(31, 0);
				instruc.set(30, 1);
				instruc.set(29, 1);
				instruc.set(28, 1);
				instruc.set(10, 1);
				instruc.set(8, 1);

				putOperand(i->op1, 1, instruc);

				target << writeIns(instruc);

			} else if (i->type == 35) {	//stro

				bitset<32> instruc(i->op2);
				instruc <<= 8;
				instruc.set(30, 1);
				instruc.set(29, 0);
				instruc.set(28, 1);
				instruc.set(27, 0);

				putOperand(i->op1, 1, instruc);

				target << writeIns(instruc);

			}

			i = i->next;

		} while (i != NULL);

	}

	target.close();

}

/**
 * Retorno a sequencia de bits especificado em uma string com a instrução em bytes
 * @param set
 * Array de bits da instrução montada
 * @return
 * String com a instrução codificada em bytes
 */
string Assembler_Dual::writeIns (bitset<32> set) {
	stringstream ss;
	ss << hex << set.to_ulong();
	string s;
	s.append("0x");
	s.append(ss.str());
	s.append("\n");
	return s;
}

/**
 * Identifica se a string espeficada contém uma determinada sequência de caracteres
 * @param str
 * String que será analizada
 * @param search
 * String usada para o teste
 * @return
 * Verdadeiro caso str contenha search
 */
bool Assembler_Dual::containWord (string str, string search) {
	for (int i = 0; i < strlen(str.c_str()); ++i) {
		if (strncmp(&str[i], search.c_str(), strlen(search.c_str())) == 0)
			return true;
	}

	return false;
}

/**
 * Adiciona o valor no operando especificado
 * @param value
 * Valor que o operando deve conter
 * @param adr
 * Operando a ser escrito
 * @parat bit
 * Array de bits da instrução que está sendo montada
 */

void Assembler_Dual::putOperand	(int value, int adr, bitset<32>& bit) {
	bitset<4> op(value);
	int i, cnt = 0;
	if (adr == 0)
		i = 0;
	else
		i = 4;
	string bits = bit.to_string();
	string ops = op.to_string();
	for (int x = i; x < i + 4; x++) {
		bit[x] = op[cnt];
		cnt++;
	}

}
