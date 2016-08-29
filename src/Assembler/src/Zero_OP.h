/*
 * Zero_OP.h
 *
 *  Created on: Aug 17, 2015
 *      Author: nuts
 */

#ifndef SRC_ZERO_OP_H_
#define SRC_ZERO_OP_H_

#define TARGET_PATH "/home/nuts/Desktop/program.sm"

#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include <bitset>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

using namespace std;

struct Instruction{

	int 			type, value, cnt;
	bool 			isReturn, noConsume;
	Instruction 	*next;

};


class Assembler_Zero {

private:
	string 			file_name;
	ifstream 		program_file;
	Instruction 	*head;
	char* 			targetPath;

public:

	Assembler_Zero();
	Assembler_Zero(char* entry[]);
	~Assembler_Zero();
	void Load_Program();
	void Print();
	int getInt(string s);
	int getType(string s);
	void writeProgram();
	string writeIns(bitset<32> set);
	bool containWord(string str, string search);

};

#endif /* SRC_ZERO_OP_H_ */
