/**********************************************************************
 * Copyright (c) 2019-2021
 *  Sang-Hoon Kim <sanghoonkim@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>

 /*====================================================================*/
 /*          ****** DO NOT MODIFY ANYTHING FROM THIS LINE ******       */

 /* To avoid security error on Visual Studio */
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

#define MAX_NR_TOKENS	32	/* Maximum length of tokens in a command */
#define MAX_TOKEN_LEN	64	/* Maximum length of single token */
#define MAX_COMMAND	256 /* Maximum length of command string */

typedef unsigned char bool;
#define true	1
#define false	0

/**
 * memory[] emulates the memory of the machine
 */
static unsigned char memory[1 << 20] = {	/* 1MB memory at 0x0000 0000 -- 0x0100 0000 */
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	0xde, 0xad, 0xbe, 0xef, 0x00, 0x00, 0x00, 0x00,
	'h',  'e',  'l',  'l',  'o',  ' ',  'w',  'o',
	'r',  'l',  'd',  '!',  '!',  0x00, 0x00, 0x00,
	'a',  'w',  'e',  's',  'o',  'm',  'e',  ' ',
	'c',  'o',  'm',  'p',  'u',  't',  'e',  'r',
	' ',  'a',  'r',  'c',  'h',  'i',  't',  'e',
	'c',  't',  'u',  'r',  'e',  '!',  0x00, 0x00,
};

#define INITIAL_PC	0x1000	/* Initial value for PC register */
#define INITIAL_SP	0x8000	/* Initial location for stack pointer */

/**
 * Registers of the machine
 */
static unsigned int registers[32] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0x10, INITIAL_PC, 0x20, 3, 0xbadacafe, 0xcdcdcdcd, 0xffffffff, 7,
	0, 0, 0, 0, 0, INITIAL_SP, 0, 0,
};

/**
 * Names of the registers. Note that $zero is shorten to zr
 */
const char* register_names[] = {
	"zr", "at", "v0", "v1", "a0", "a1", "a2", "a3",
	"t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
	"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
	"t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"
};

/**
 * Program counter register
 */
static unsigned int pc = INITIAL_PC;

/**
 * strmatch()
 *
 * DESCRIPTION
 *   Compare strings @str and @expect and return 1 if they are the same.
 *   You may use this function to simplify string matching :)
 *
 * RETURN
 *   1 if @str and @expect are the same
 *   0 otherwise
 */
static inline bool strmatch(char* const str, const char* expect)
{
	return (strlen(str) == strlen(expect)) && (strncmp(str, expect, strlen(expect)) == 0);
}

/*          ****** DO NOT MODIFY ANYTHING UP TO THIS LINE ******      */
/*====================================================================*/

enum format { r, i, j }; // 각 foramt을 정수로 변환( r = 0 , i = 1 , j = 2 )

#define operationList_size 0x40
#define operation_size 5

/*
* MIPS Operation을 저장하는 배열. idx는 각 operation의 opcode
*/
static unsigned char** operationList;

void makeOPlist() { // operation 배열을 초기화
	operationList = (char**)malloc(sizeof(char*) * operationList_size);
	for (int i = 0; i < operationList_size; i++)
		operationList[i] = (char*)malloc(sizeof(char) * operation_size);

	operationList[0x08] = "addi"; operationList[0x0c] = "andi"; operationList[0x0d] = "ori"; operationList[0x23] = "lw"; operationList[0x2b] = "sw";
	operationList[0x0a] = "slti"; operationList[0x04] = "beq"; operationList[0x05] = "bne"; operationList[0x02] = "j"; operationList[0x03] = "jal";
	operationList[0x3f] = "halt"; operationList[0x30] = "add"; operationList[0x32] = "sub"; operationList[0x34] = "and"; operationList[0x35] = "or";
	operationList[0x37] = "nor"; operationList[0x10] = "sll"; operationList[0x12] = "srl"; operationList[0x13] = "sra"; operationList[0x3a] = "slt";
	operationList[0x18] = "jr";
}

/*
* MIPS operation에 대응하는 type을 저장하는 배열. idx는 해당 operation의 opcode
*/
static unsigned char* typeList;

void makeTypeList() { // typeList 배열을 초기화
	typeList = (char*)malloc(sizeof(char) * operationList_size);

	typeList[0x08] = i; typeList[0x0c] = i; typeList[0x0d] = i; typeList[0x23] = i; typeList[0x2b] = i;
	typeList[0x0a] = i; typeList[0x04] = i; typeList[0x05] = i; typeList[0x02] = j; typeList[0x03] = j;
	typeList[0x18] = r; typeList[0x30] = r; typeList[0x32] = r; typeList[0x34] = r; typeList[0x35] = r;
	typeList[0x37] = r; typeList[0x30] = r; typeList[0x32] = r; typeList[0x34] = r; typeList[0x3a] = r;
	typeList[0x3f] = -1;
}

/**
 * process_operation
 *
 * DESCRIPTION
 *   operation에 맞게 instruction을 수행한다
 * 
	 parameters: 
 *	 (1) operation : instruction의 operation 
 *   (2) values : instruction format을 구성하는 모든 filed 값들
 *   (3) type : instruction의 type
 *  
 */
void process_operation(char* operation, int* values, int type) {

	/// R: values[0]=rs, values[1]=rt, values[2]=rd, values[3]=shamt, values[4]=funct
	/// I: values[0]=rs, values[1]=rt, values[3]=immediate value
	/// J: values[0]=address

	int rs = 0, rt = 0, rd = 0;
	int shamt = 0, funct = 0, value = 0, addr = 0;
	unsigned int temp = 0;

	if (type == 0) { //R
		rs = values[0];
		rt = values[1];
		rd = values[2];
		shamt = values[3];
		funct = values[4];
	}
	else if (type == 1) { //I
		rs = values[0];
		rt = values[1];
		value = values[2];

	}
	else if (type == 2) //J
		addr = values[0];

	if (!strcmp(operation, "add")) { //R
		registers[rd] = registers[rs] + registers[rt];
	}
	else if (!strcmp(operation, "addi")) { //I
		if (rs == 29 && value >= 0) { //$ra
			for (int i = 0; i < value; i++)
				memory[registers[rs] + i] = 0;
		}
		registers[rt] = registers[rs] + value;
	}
	else if (!strcmp(operation, "sub")) { //R
		registers[rd] = registers[rs] - registers[rt];
	}
	else if (!strcmp(operation, "and")) { //R
		registers[rd] = registers[rs] & registers[rt];
	}
	else if (!strcmp(operation, "andi")) { //I
		registers[rt] = registers[rs] & value;
	}
	else if (!strcmp(operation, "or")) { //R
		registers[rd] = registers[rs] | registers[rt];
	}
	else if (!strcmp(operation, "ori")) { //I
		registers[rt] = registers[rs] | value;
	}
	else if (!strcmp(operation, "nor")) { //R
		registers[rd] = ~(registers[rs] | registers[rt]);
	}
	else if (!strcmp(operation, "sll")) { //R
		registers[rd] = registers[rt] << shamt;
	}
	else if (!strcmp(operation, "srl")) { //R
		registers[rd] = registers[rt] >> shamt;
	}
	else if (!strcmp(operation, "sra")) { //R
		if (registers[rt] >> 31 == 1) {
			int shamt_tp = 32 - shamt;
			registers[rd] = (registers[rt] >> shamt) + (0xffffffff << shamt_tp);
		}
		else
			registers[rd] = registers[rt] >> shamt;
	}
	else if (!strcmp(operation, "lw")) { //I
		temp = registers[rs] + value;

		unsigned int first = (unsigned int)memory[temp];
		unsigned int second = (unsigned int)memory[temp + 1];
		unsigned int third = (unsigned int)memory[temp + 2];
		unsigned int fourth = (unsigned int)memory[temp + 3];

		temp = (((first / 16) << 28) + ((first % 16) << 24) + ((second / 16) << 20) + ((second % 16) << 16) + ((third / 16) << 12) + ((third % 16) << 8) + ((fourth / 16) << 4) + (fourth % 16));

		registers[rt] = temp;
		temp = registers[rs] + value;
	}
	else if (!strcmp(operation, "sw")) { //I
		temp = registers[rs] + value;
		int target = registers[rt];

		unsigned int first = ((target >> 24) & 0x000000ff);
		unsigned int second = ((target >> 16) & 0x000000ff);
		unsigned int third = ((target >> 8) & 0x000000ff);
		unsigned int fourth = (target & 0x000000ff);

		memory[temp] = (first / 16) * 16 + (first % 16);
		memory[temp + 1] = (second / 16) * 16 + (second % 16);
		memory[temp + 2] = (third / 16) * 16 + (third % 16);
		memory[temp + 3] = (fourth / 16) * 16 + (fourth % 16);
	}
	else if (!strcmp(operation, "slt")) { //R
		if ((signed)registers[rs] < (signed)registers[rt])
			registers[rd] = 1;
		else
			registers[rd] = 0;
	}
	else if (!strcmp(operation, "slti")) { //I
		if ((signed)registers[rs] < value)
			registers[rt] = 1;
		else
			registers[rt] = 0;
	}
	else if (!strcmp(operation, "beq")) { //I
		if (registers[rs] == registers[rt]) {
			pc += (value << 2);
		}
	}
	else if (!strcmp(operation, "bne")) { //I
		if (registers[rs] != registers[rt]) {
			pc += (value << 2);
		}
	}
	else if (!strcmp(operation, "jr")) { //R
		pc = registers[rs];
	}
	else if (!strcmp(operation, "j")) { //J
		temp = ((pc >> 26) & 0x0000003c); //현재 pc값의 상위 4bit
		pc = (addr << 2) + (temp << 26); //word -> address
	}
	else if (!strcmp(operation, "jal")) { //J
		temp = ((pc >> 26) & 0x0000003c); //현재 pc값의 상위 4bit
		registers[31] = pc; // 현재 pc값을 ra에 저장
		pc = (addr << 2) + (temp << 26); //word -> address
	}
}

/**********************************************************************
 * process_instruction
 *
 * DESCRIPTION
 *   Execute the machine code given through @instr. The following table lists
 *   up the instructions to support. Note that a pseudo instruction 'halt'
 *   (0xffffffff) is added for the testing purpose. Also '*' instrunctions are
 *   the ones that are newly added to PA2.
 *
 * | Name   | Format    | Opcode / opcode + funct |
 * | ------ | --------- | ----------------------- |
 * | `add`  | r-format  | 0 + 0x20                |
 * | `addi` | i-format  | 0x08                    |
 * | `sub`  | r-format  | 0 + 0x22                |
 * | `and`  | r-format  | 0 + 0x24                |
 * | `andi` | i-format  | 0x0c                    |
 * | `or`   | r-format  | 0 + 0x25                |
 * | `ori`  | i-format  | 0x0d                    |
 * | `nor`  | r-format  | 0 + 0x27                |
 * | `sll`  | r-format  | 0 + 0x00                |
 * | `srl`  | r-format  | 0 + 0x02                |
 * | `sra`  | r-format  | 0 + 0x03                |
 * | `lw`   | i-format  | 0x23                    |
 * | `sw`   | i-format  | 0x2b                    |
 * | `slt`  | r-format* | 0 + 0x2a                |
 * | `slti` | i-format* | 0x0a                    |
 * | `beq`  | i-format* | 0x04                    |
 * | `bne`  | i-format* | 0x05                    |
 * | `jr`   | r-format* | 0 + 0x08                |
 * | `j`    | j-format* | 0x02                    |
 * | `jal`  | j-format* | 0x03                    |
 * | `halt` | special*  | @instr == 0xffffffff    |
 *
 * RETURN VALUE
 *   1 if successfully processed the instruction.
 *   0 if @instr is 'halt' or unknown instructions
 */
static int process_instruction(unsigned int instr)
{
	//----------------------------- 10진수 instruction -> 8자리 16진수로 변환

	char oxinstruction[9]; // 16진수 instruction
	int decimal;
	int div;

	int idx = 7;
	while (instr > 0) {
		div = instr % 16;
		if (div < 10)
			oxinstruction[idx--] = 48 + div;
		else
			oxinstruction[idx--] = 97 + (div - 10);

		instr /= 16;
	}

	while (idx >= 0)
		oxinstruction[idx--] = '0';
	oxinstruction[8] = '\0';

	if (!strcmp(oxinstruction, "ffffffff")) { //halt를 만나면 종료
		return 0;
	}

	//----------------------------- 8자리 8진수 -> 32자리 2진수로 변환

	char binstruction[32];
	int num;

	for (int i = 7, k = 28, idx = 31; i >= 0; i--, k -= 4) {
		if (oxinstruction[i] <= 57)
			num = oxinstruction[i] - '0';
		else
			num = oxinstruction[i] - 'a' + 10;

		while (num > 0) {
			binstruction[idx--] = num % 2;
			num /= 2;
		}

		while (idx >= k)
			binstruction[idx--] = 0;
	}

	//------------------------------- 32자리 2진수의 앞 6bit로 opcode 판별

	char* operation;
	int rs = 0, rt = 0, rd = 0;
	int shamt = 0, funct = 0, value = 0, addr = 0;

	int opcode = 0;
	for (int i = 5, mul = 1; i >= 0; i--, mul <<= 1)
		opcode += binstruction[i] * mul;

	if (opcode == 0) {//R_format
		for (int i = 31, mul = 1; i >= 26; i--, mul <<= 1) //R_format은 opcode = 0이니 funct로 판별
			funct += binstruction[i] * mul;
		operation = operationList[funct + 16];
	}
	else //I_format, J_format
		operation = operationList[opcode];

	int type=0;

	for (int i = 0; i < 0x40; i++) {
		if (!strcmp(operation, operationList[i])) {
			type = typeList[i];
			break;
		}
	}

	//------------------------------ opcode를 이용해 format을 판별한 뒤, 각 format에 맞게 instruction field를 채움

	int values[5] = { 0 }; //filed 값을 저장

	/// R: values[0]=rs, values[1]=rt, values[2]=rd, values[3]=shamt, values[4]=funct
	/// I: values[0]=rs, values[1]=rt, values[3]=immediate value
	/// J: values[0]=address

	if (type == r) {
		for (int i = 10, mul = 1; i >= 6; i--, mul <<= 1)
			rs += binstruction[i] * mul;
		for (int i = 15, mul = 1; i >= 11; i--, mul <<= 1)
			rt += binstruction[i] * mul;
		for (int i = 20, mul = 1; i >= 16; i--, mul <<= 1)
			rd += binstruction[i] * mul;
		for (int i = 25, mul = 1; i >= 21; i--, mul <<= 1)
			shamt += binstruction[i] * mul;

		values[0] = rs, values[1] = rt, values[2] = rd, values[3] = shamt, values[4] = funct;
	}
	else if (type == i) {
		for (int i = 10, mul = 1; i >= 6; i--, mul <<= 1)
			rs += binstruction[i] * mul;
		for (int i = 15, mul = 1; i >= 11; i--, mul <<= 1)
			rt += binstruction[i] * mul;

		int neg = false;
		if ( (binstruction[16] == 1) && !( !(strcmp(operation,"andi")) || !(strcmp(operation, "ori"))))
			neg = true;
		for (int i = 31, mul = 1; i >= 16; i--, mul <<= 1) {
			if (neg)
				value += !binstruction[i] * mul;
			else
				value += binstruction[i] * mul;
		}

		if (neg)
			value = (value * -1) - 1;

		values[0] = rs, values[1] = rt, values[2] = value;
	}
	else if (type == j) {
		for (int i = 31, mul = 1; i >= 6; i--, mul <<= 1)
			addr += binstruction[i] * mul;

		values[0] = addr;
	}

	//------------------------------

	process_operation(operation, values, type); // 분해한 instruction을 operation에 맞게 실행

	return 1;
}

/**********************************************************************
 * load_program
 *
 * DESCRIPTION
 *   Load the instructions in the file @filename onto the memory starting at
 *   @INITIAL_PC. Each line in the program file looks like;
 *
 *	 [MIPS instruction started with 0x prefix]  // optional comments
 *
 *   For example,
 *
 *   0x8c090008
 *   0xac090020	// sw t1, zero + 32
 *   0x8c080000
 *
 *   implies three MIPS instructions to load. Each machine instruction may
 *   be followed by comments like the second instruction. However you can simply
 *   call strtoimax(linebuffer, NULL, 0) to read the machine code while
 *   ignoring the comment parts.
 *
 *	 The program DOES NOT include the 'halt' instruction. Thus, make sure the
 *	 'halt' instruction is appended to the loaded instructions to terminate
 *	 your program properly.
 *
 *	 Refer to the @main() for reading data from files. (fopen, fgets, fclose).
 *
 * RETURN
 *	 0 on successfully load the program
 *	 any other value otherwise
 */

static int __parse_command(char* command, int* nr_tokens, char* tokens[]);

static int load_program(char* const filename)
{
	FILE* input = fopen(filename, "r"); //input stream을 통해 file을 읽어옴
	if (!input) {
		printf("file open error!\n");
		return -EINVAL;
	}

	char command[MAX_COMMAND] = { '\0' };
	
	while (fgets(command, sizeof(command), input)) {
		char* tokens[MAX_NR_TOKENS] = { NULL };
		int nr_tokens = 0;
		for (size_t i = 0; i < strlen(command); i++) {
			command[i] = tolower(command[i]);
		}

		if (__parse_command(command, &nr_tokens, tokens) < 0)
			continue;

		char* token = tokens[0];
		char hexa[8] = { 0 };

		for (int i = 2; i <= 9; i++) {
			if (token[i] <= 57)
				token[i] = token[i] - '0';
			else
				token[i] = token[i] - 'a' + 10;
		}

		for (int i = 3, k = 9; i >= 0; i--, k -= 2) 
			memory[pc + i] = token[k-1] * 16 + token[k];

		pc += 4;
	}

	for (int i = 3; i >= 0; i--)
		memory[pc + i] = 0xff;

	return 0;
}

/**********************************************************************
 * run_program
 *
 * DESCRIPTION
 *   Start running the program that is loaded by @load_program function above.
 *   If you implement @load_program() properly, the first instruction is placed
 *   at @INITIAL_PC. Using @pc, which is the program counter of this processor,
 *   you can emulate the MIPS processor by
 *
 *   1. Read instruction from @pc
 *   2. Increment @pc by 4
 *   3. Call @process_instruction(instruction)
 *   4. Repeat until @process_instruction() returns 0
 *
 * RETURN
 *   0
 */
static int run_program(void)
{
	pc = INITIAL_PC;

	while (true) {
		char hexa[8];
		int decimal;
		int div;

		for (int i = 0, k = 0; i < 4; i++) { //2자리씩 4번 저장된 16진수 -> 8자리 16진수
			decimal = memory[pc + i];

			div = decimal / 16;
			if (div < 10)
				hexa[k++] = 48 + div;
			else
				hexa[k++] = 97 + (div - 10);

			div = decimal % 16;
			if (div < 10)
				hexa[k++] = 48 + div;
			else
				hexa[k++] = 97 + (div - 10);
		}

		unsigned int instruction = strtoimax(hexa, NULL, 16);

		pc += 4;

		int flag = process_instruction(instruction);
		if (!flag)  //meet halt
			break;
	}

	return 0;
}

/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING FROM THIS LINE ******       */
static void __show_registers(char* const register_name)
{
	int from = 0, to = 0;
	bool include_pc = false;

	if (strmatch(register_name, "all")) {
		from = 0;
		to = 32;
		include_pc = true;
	}
	else if (strmatch(register_name, "pc")) {
		include_pc = true;
	}
	else {
		for (int i = 0; i < sizeof(register_names) / sizeof(*register_names); i++) {
			if (strmatch(register_name, register_names[i])) {
				from = i;
				to = i + 1;
			}
		}
	}

	for (int i = from; i < to; i++) {
		fprintf(stderr, "[%02d:%2s] 0x%08x    %u\n", i, register_names[i], registers[i], registers[i]);
	}
	if (include_pc) {
		fprintf(stderr, "[  pc ] 0x%08x\n", pc);
	}
}

static void __dump_memory(unsigned int addr, size_t length)
{
	for (size_t i = 0; i < length; i += 4) {
		fprintf(stderr, "0x%08lx:  %02x %02x %02x %02x    %c %c %c %c\n",
			addr + i,
			memory[addr + i], memory[addr + i + 1],
			memory[addr + i + 2], memory[addr + i + 3],
			isprint(memory[addr + i]) ? memory[addr + i] : '.',
			isprint(memory[addr + i + 1]) ? memory[addr + i + 1] : '.',
			isprint(memory[addr + i + 2]) ? memory[addr + i + 2] : '.',
			isprint(memory[addr + i + 3]) ? memory[addr + i + 3] : '.');
	}
}

static void __process_command(int argc, char* argv[])
{
	if (argc == 0) return;
	if (strmatch(argv[0], "load")) {
		if (argc == 2) {
			load_program(argv[1]);
		}
		else {
			printf("Usage: load [program filename]\n");
		}
	}
	else if (strmatch(argv[0], "run")) {
		if (argc == 1) {
			run_program();
		}
		else {
			printf("Usage: run\n");
		}
	}
	else if (strmatch(argv[0], "show")) {
		if (argc == 1) {
			__show_registers("all");
		}
		else if (argc == 2) {
			__show_registers(argv[1]);
		}
		else {
			printf("Usage: show { [register name] }\n");
		}
	}
	else if (strmatch(argv[0], "dump")) {
		if (argc == 3) {
			__dump_memory(strtoimax(argv[1], NULL, 0), strtoimax(argv[2], NULL, 0));
		}
		else {
			printf("Usage: dump [start address] [length]\n");
		}
	}
	else {
		/**
		 * You may hook up @translate() from pa1 here to allow assembly input!
		 */
#ifdef INPUT_ASSEMBLY
		unsigned int instr = translate(argc, argv);
		process_instruction(instr);
#else
		process_instruction(strtoimax(argv[0], NULL, 0));
#endif
	}
}

static int __parse_command(char* command, int* nr_tokens, char* tokens[])
{
	char* curr = command;
	int token_started = false;
	*nr_tokens = 0;

	while (*curr != '\0') {
		if (isspace(*curr)) {
			*curr = '\0';
			token_started = false;
		}
		else {
			if (!token_started) {
				tokens[*nr_tokens] = curr;
				*nr_tokens += 1;
				token_started = true;
			}
		}
		curr++;
	}

	/* Exclude comments from tokens */
	for (int i = 0; i < *nr_tokens; i++) {
		if (strmatch(tokens[i], "//") || strmatch(tokens[i], "#")) {
			*nr_tokens = i;
			tokens[i] = NULL;
		}
	}

	return 0;
}

int main(int argc, char* const argv[])
{
	char command[MAX_COMMAND] = { '\0' };
	//FILE* input = stdin;
	FILE* input = fopen("C:\\Users\\dnwls\\input_file\\input.txt","r");

	if (argc > 1) {
		input = fopen(argv[1], "r");
		if (!input) {
			fprintf(stderr, "No input file %s\n", argv[0]);
			return EXIT_FAILURE;
		}
	}

	if (input == stdin) {
		printf("*********************************************************\n");
		printf("*          >> SCE212 MIPS Simulator v0.01 <<            *\n");
		printf("*                                                       *\n");
		printf("*                                       .---.           *\n");
		printf("*                           .--------.  |___|           *\n");
		printf("*                           |.------.|  | =.|           *\n");
		printf("*                           || >>_  ||  |---|           *\n");
		printf("*                           |'------'|  |   |           *\n");
		printf("*                           ')______('~~|___|           *\n");
		printf("*                                                       *\n");
		printf("*                                   Fall 2021           *\n");
		printf("*********************************************************\n\n");
		printf(">> ");
	}

	makeOPlist();
	makeTypeList();

	while (fgets(command, sizeof(command), input)) {
		char* tokens[MAX_NR_TOKENS] = { NULL };
		int nr_tokens = 0;

		for (size_t i = 0; i < strlen(command); i++) {
			command[i] = tolower(command[i]);
		}

		if (__parse_command(command, &nr_tokens, tokens) < 0)
			continue;

		__process_command(nr_tokens, tokens);

		if (input == stdin) printf(">> ");
	}

	if (input != stdin) fclose(input);

	return EXIT_SUCCESS;
}