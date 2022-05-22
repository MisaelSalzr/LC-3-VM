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

enum
{
	TRAP_GETC = 0x20,  /* get character from keyboard, not echoed onto the terminal */
	TRAP_OUT = 0x21,   /* output a character */
	TRAP_PUTS = 0x22,  /* output a word string */
	TRAP_IN = 0x23,    /* get character from keyboard, echoed onto the terminal */
	TRAP_PUTSP = 0x24, /* output a byte string */
	TRAP_HALT = 0x25   /* halt the program */
};

uint16_t sign_extend(uint16_t value, int bit_count);
void update_flags(uint16_t r);

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
			{

			//Destination register
			uint16_t r0 = (instr >> 9) & 0x7;
			//First operand register
			uint16_t r1 = (instr >> 6) & 0x7;
			//Check if we're in immediate mode
			uint16_t imm_flag = (instr >> 5) & 0x1;

			if (imm_flag) {

				uint16_t imm5 = sign_extend(instr & 0x1F, 5);
				reg[r0] = reg[r1] + imm5;
			}
			else {

				uint16_t r2 = instr & 0x7;
				reg[r0] = reg[r1] + reg[r2];
			}
			update_flags(r0);

			}
			break;
		case OP_AND:
			{

			uint16_t r0 = (instr >> 9) & 0x7;
			uint16_t r1 = (instr >> 6) & 0x7;
			uint16_t imm_flag = (instr >> 5) & 0x1;

			if (imm_flag) {
				
				uint16_t imm5 = sign_extend(instr & 0x1F, 5);
				reg[r0] = reg[r1] & imm5;
			}
			else {
				uint16_t r2 = instr & 0x7;
				reg[r0] = reg[r1] & reg[r2];
			}
			update_flags(r0);
		
			}
			break;
		case OP_NOT:
			{

			uint16_t r0 = (instr >> 9) & 0x7;
			uint16_t r1 = (instr >> 6) & 0x7;

			reg[r0] = ~reg[r1];
			update_flags(r0);

			}
			break;
		case OP_BR:
			{
			
			uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
			uint16_t cond_flag = (instr >> 9) & 0x7;
			if (cond_flag & reg[R_COND]) {
				reg[R_PC] += pc_offset;
			}

			}
			break;
		case OP_JMP:
			{
		
			uint16_t r1 = (instr >> 6) & 0x7;
			reg[R_PC] = reg[r1];

			}
			break;
		case OP_JSR:
			{
		
			uint16_t long_flag = (instr >> 11) & 1;
			reg[R_R7] = reg[R_PC];
			if (long_flag) {

				uint16_t long_pc_offset = sign_extend(instr & 0x7FF, 11);
				reg[R_PC] += long_pc_offset;
			}
			else {

				uint16_t r1 = (instr >> 6) & 0x7;
				reg[R_PC] = reg[r1];
			}

			}
			break;
		case OP_LD:
			{
		
			uint16_t r0 = (instr >> 9) & 0x7;
			uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
			reg[r0] = mem_read(reg[R_PC] + pc_offset);
			update_flags(r0);
			}
			break;
		case OP_LDI:
			{
			//Destination register
			uint16_t r0 = (instr >> 9) & 0x7;
			//PC offset by 9
			uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
		
			reg[r0] = mem_read(mem_read(reg[R_PC] + pc_offset));
			update_flags(r0);
			}
			break;
		case OP_LDR:
			{
		
			uint16_t r0 = (instr >> 9) & 0x7;
			uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
			reg[r0] = mem_read(reg[R_PC] + pc_offset);
			update_flags(r0);
			}
			break;
		case OP_LEA:
			{
		
			uint16_t r0 = (instr >> 9) & 0x7;
			uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
			reg[r0] = reg[R_PC] + pc_offset;
			update_flags(r0);
			}
			break;
		case OP_ST:
			{
		
			uint16_t r0 = (instr >> 9) & 0x7;
			uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
			mem_write(reg[R_PC] + pc_offset, reg[r0]);
			}
			break;
		case OP_STI:
			{
		
			uint16_t r0 = (instr >> 9) & 0x7;
			uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
			mem_write(mem_read(reg[R_PC] + pc_offset), reg[r0]);
			}
			break;
		case OP_STR:
			{
		
			uint16_t r0 = (instr >> 9) & 0x7;
			uint16_t r1 = (instr >> 6) & 0x7;
			uint16_t offset = sign_extend(instr & 0x3F, 6);
			mem_write(reg[r1] + offset, reg[r0]);
			}
			break;
		case OP_TRAP:
			reg[R_R7] = reg[R_PC];

			switch (instr & 0xFF) {

			case TRAP_GETC:
				
				break;
			case TRAP_OUT:

				break;
			case TRAP_PUTS:

				break;
			case TRAP_IN:

				break;
			case TRAP_PUTSP:

				break;
			case TRAP_HALT:

				break;

			}
			break;
		case OP_RES:
		case OP_RTI:
		default:
			abort();
			break;

		}

	}

	//end

}

uint16_t sign_extend(uint16_t value, int bit_count) {

	if ((value >> (bit_count - 1)) & 1) {
		value |= (0xFFFF << bit_count);
	}
	return value;
}

void update_flags(uint16_t r) {

	if (reg[r] == 0) {
		reg[R_COND] = FL_ZRO;
	}
	else if (reg[r] >> 15) { //If the left-most bit is 1, it indicates a negative value
		reg[R_COND] = FL_NEG;
	}
	else {
		reg[R_COND] = FL_POS;
	}

}