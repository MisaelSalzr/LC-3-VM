#include <stdio.h>
#include <stdint.h>
#include <signal.h>

#include <Windows.h>
#include <conio.h>

#define MEMORY_MAX (1 << 16)
uint16_t memory[MEMORY_MAX]; //2^16 = 65536 memory locations

//There are 10 registers, stored in an array
enum {
	R_RO = 0,
	R_R1,
	R_R2,
	R_R3,
	R_R4,
	R_R5,
	R_R6,
	R_R7,
	R_PC,	//Program Counter register
	R_COND,
	R_COUNT
};

uint16_t reg[R_COUNT];

//The Instruction Set:
enum {
	OP_BR = 0, //Branch
	OP_ADD,	//Add
	OP_LD, //Load
	OP_ST, //Store
	OP_JSR,	//Jump register
	OP_AND,	//Bitwise AND
	OP_LDR, //Load register
	OP_STR,	//Store register
	OP_RTI, //Unused
	OP_NOT, //Bitwise NOT
	OP_LDI, //Load indirect
	OP_STI, //Store indirect
	OP_JMP, //Jump
	OP_RES, //Reserved
	OP_LEA, //Load effective address
	OP_TRAP
};

//Condition flags
enum {
	FL_POS = 1 << 0, //P
	FL_ZRO = 1 << 1, //Z
	FL_NEG = 1 << 2, //N
};


int main(int argc, const char* argv[]) {

	if (argc < 2) {
		printf("lc3 [image-file1] ...\n");
		exit(2);
	}

	for (int j = 1; j < argc; ++j) {
		if (!read_image(argv[j])) {
			printf("failed to load image: %s\n", argv[j]);
			exit(1);
		}
	}

	//Setup

	//Only one condition flag should be set at a time, set Z flag
	reg[R_COND] = FL_ZRO;

	//Set the PC to starting position
	enum { PC_START = 0x3000 };
	reg[R_PC] = PC_START;

	int running = 1;
	while (running) {

		//Fetch
		uint16_t instr = mem_read(reg[R_PC]++);
		uint16_t op = instr >> 12;

		switch (op) {

		case OP_ADD:
			//
			break;
		case OP_AND:
			//
			break;
		case OP_NOT:
			//
			break;
		case OP_BR:
			//
			break;
		case OP_JMP:
			//
			break;
		case OP_JSR:
			//
			break;
		case OP_LD:
			//
			break;
		case OP_LDI:
			//
			break;
		case OP_LDR:
			//
			break;
		case OP_LEA:
			//
			break;
		case OP_ST:
			//
			break;
		case OP_STI:
			//
			break;
		case OP_STR:
			//
			break;
		case OP_TRAP:
			//
			break;
		case OP_RES:
		case OP_RTI:
		default:
			//
			break;

		}

	}

	//end

}



