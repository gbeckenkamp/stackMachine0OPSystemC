/*
 * main.cpp
 *
 *  Created on: Aug 17, 2015
 *      Author: nuts
 */

#include "Zero_OP.h"
//#include "Dual_OP.h"


int main(int argc, char* argv[]){

	Assembler_Zero *az;
//	Assembler_Dual *az;
	if(argc != 3)
		cout<<"Especifique um arquivo de entrada e um arquivo de saida.\n";
	else{
//		az = new Assembler_Dual(argv);
		az = new Assembler_Zero(argv);
		az->Load_Program();
		az->writeProgram();
	}

	delete(az);

}
