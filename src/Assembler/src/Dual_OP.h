/*
 * Dual_OP.h
 *
 *  Created on: Aug 20, 2015
 *      Author: nuts
 */

#ifndef SRC_DUAL_OP_H_
#define SRC_DUAL_OP_H_

#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include <bitset>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string>

using namespace std;

struct Instruction {
	int 			type, cnt, op1, op2, op3;
	bool 			isReturn;
	Instruction 	*next;
};

class Assembler_Dual {
	string 			file_name;
	ifstream 		program_file;
	Instruction 	*head;
	char* 			targetPath;

public:

	Assembler_Dual();
	Assembler_Dual(char* entry[]);
	~Assembler_Dual();
	void Load_Program();
	void Print();
	int getInt(string s);
	int getType(string s);
	void writeProgram();
	string writeIns(bitset<32> set);
	bool containWord(string str, string search);
	void putOperand(int value, int adr, bitset<32>& bit);
};

#endif /* SRC_DUAL_OP_H_ */
