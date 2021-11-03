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

#define MAP_SIZE 100
#define WORD_LEN 5
enum slotStatus { empty, deleted, inuse };
enum format {r,i,j};
typedef int (*hashfunc)(char* key);

typedef struct Slot { //Hash Map을 이루는 각 slot
	char key[WORD_LEN];
	int* val;
	enum slotStatus status;
}slot;

typedef struct HashMap { //Hash Map
	slot bucket[MAP_SIZE];
	hashfunc hf;
}map;

map hm;

void mapInit(map* hm, hashfunc hf) {
	for (int i = 0; i < MAP_SIZE; i++)
		(hm->bucket[i]).status = empty;
	hm->hf = hf;
}

int makeHash(char* str) { //인자로 전달된 key값을 hash value로 전환
	int hash = 0;
	while (*str != '\0') {
		hash += (int)(*str) % MAP_SIZE;
		str++;
	}

	return hash % MAP_SIZE;
}

void insert(map* hm, char* key, int val1, int val2, int val3) { //Hash Map에 인자로 전달된 (key, value)를 저장
	int* arr = (int*)malloc(sizeof(int) * 3);
	arr[0] = val1; arr[1] = val2; arr[2] = val3;
	int hv = hm->hf(key);

	while (hm->bucket[hv].status == inuse) {
		hv++;
		hv %= MAP_SIZE;
	}

	hm->bucket[hv].val = arr;
	strcpy(hm->bucket[hv].key, key);
	hm->bucket[hv].status = inuse;
}

int* search(map* hm, char* key) { // Hash Map에서 인자로 전달된 key에 대응되는 value을 반환
	int hv = hm->hf(key);
	int temp = hv;
	while (strcmp(key, hm->bucket[hv].key)) {
		hv++;
		hv %= MAP_SIZE;

		if (hv == temp)
			return NULL;
	}

	return hm->bucket[hv].val;
}

void makeMap(map* hm) { // Hash Map 초기화 함수(opcode/funct number, register number)
	mapInit(hm, makeHash);
	int* arr;

	// opcode/funct
	insert(hm, "add", 0, 0x20, r); insert(hm, "addi", 0x08, 0, i); insert(hm, "andi", 0x0c, 0, i); insert(hm, "or", 0, 0x25, r);
	insert(hm, "sub", 0, 0x22, r); insert(hm, "and", 0, 0x24, r); insert(hm, "ori", 0x0d, 0, i); insert(hm, "nor", 0, 0x27, r);
	insert(hm, "sll", 0, 0x00, r); insert(hm, "srl", 0, 0x02, r); insert(hm, "sra", 0, 0x03, r); insert(hm, "lw", 0x23, 0, i);
	insert(hm, "sw", 0x2b, 0, i); insert(hm, "slt", 0, 0x2a, r); insert(hm, "slti", 0x0a, 0, i); insert(hm, "beq", 0x04, 0, i);
	insert(hm, "bne", 0x05, 0, i); insert(hm, "jr", 0, 0x08, r); insert(hm, "j", 0x02, 0, j); insert(hm, "jal", 0x03, 0, j);
	insert(hm, "halt", 0x3f, 0, -1);

	// register number
	insert(hm, "zero", 0, 0, 0); insert(hm, "at", 1, 0, 0); insert(hm, "v0", 2, 0, 0); insert(hm, "v1", 3, 0, 0);
	insert(hm, "a0", 4, 0, 0); insert(hm, "a1", 5, 0, 0); insert(hm, "a2", 6, 0, 0); insert(hm, "a3", 7, 0, 0);
	insert(hm, "t0", 8, 0, 0); insert(hm, "t1", 9, 0, 0); insert(hm, "t2", 10, 0, 0); insert(hm, "t3", 11, 0, 0);
	insert(hm, "t4", 12, 0, 0); insert(hm, "t5", 13, 0, 0); insert(hm, "t6", 14, 0, 0); insert(hm, "t7", 15, 0, 0);
	insert(hm, "s0", 16, 0, 0); insert(hm, "s1", 17, 0, 0); insert(hm, "s2", 18, 0, 0); insert(hm, "s3", 19, 0, 0);
	insert(hm, "s4", 20, 0, 0); insert(hm, "s5", 21, 0, 0); insert(hm, "s6", 22, 0, 0); insert(hm, "s7", 23, 0, 0);
	insert(hm, "t8", 24, 0, 0); insert(hm, "t9", 25, 0, 0); insert(hm, "k1", 26, 0, 0); insert(hm, "k2", 27, 0, 0);
	insert(hm, "gp", 28, 0, 0); insert(hm, "sp", 29, 0, 0); insert(hm, "fp", 30, 0, 0); insert(hm, "ra", 31, 0, 0);
}

#define oplist_size 100
#define op_size 5

static unsigned char** op;

void makeOPlist() {
	op = (char**)malloc(sizeof(char*) * oplist_size);
	for (int i = 0; i < oplist_size; i++)
		op[i] = (char*)malloc(sizeof(char) * op_size);

	op[0x08] = "addi"; op[0x0c] = "andi"; op[0x0d] = "ori"; op[0x23] = "lw"; op[0x2b] = "sw";
	op[0x0a] = "slti"; op[0x04] = "beq"; op[0x05] = "bne"; op[0x02] = "j"; op[0x03] = "jal"; 
	op[0x3f] = "halt"; op[0x30] = "add"; op[0x32] = "sub"; op[0x34] = "and"; op[0x35] = "or";
	op[0x37] = "nor"; op[0x10] = "sll"; op[0x12] = "srl"; op[0x13] = "sra"; op[0x3a] = "slt";
	op[0x18] = "jr";
}

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
	//----------------------------- 2자리씩 4번 저장된 10진수 -> 8자리 16진수

	unsigned char oxinstruction[9];
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

	if (!strcmp(oxinstruction, "ffffffff"))
		return 0;

	printf("0x");
	for (int i = 0; i < 8; i++)
		printf("%c", oxinstruction[i]);

	printf("\n");
	
	//----------------------------- 8자리 8진수 -> 32자리 2진수

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

	printf("b");
	for (int i = 0; i <= 31; i++)
		printf("%d", binstruction[i]);

	printf("\n");

	//------------------------------- 앞 6bit로 opcode 판별

	char* opcode;
	int funct = 0;
	int rs = 0, rt = 0, rd = 0;
	int shamt = 0, value = 0, addr = 0;

	int temp = 0;
	for (int i = 5, mul = 1; i >= 0; i--, mul <<= 1)
		temp += binstruction[i] * mul;

	if (temp == 0) {//R_format
		for (int i = 31, mul = 1; i >= 26; i--, mul <<= 1)
			funct += binstruction[i] * mul;
		opcode = op[funct + 16];
	}
	else //I_format, J_format
		opcode = op[temp];

	int* hm_value = search(&hm, opcode);
	printf("opcode: %s\n", opcode);

	//------------------------------ opcode를 이용해 format을 판별한 뒤, 각 format에 맞게 instruction field를 채움

	int type = hm_value[2];

	printf("type: %d\n", type);

	if (type == r) {
		for (int i = 10, mul = 1; i >= 6; i--, mul <<= 1)
			rs += binstruction[i] * mul;
		for (int i = 15, mul = 1; i >= 11; i--, mul <<= 1)
			rt += binstruction[i] * mul;
		for (int i = 20, mul = 1; i >= 16; i--, mul <<= 1)
			rd += binstruction[i] * mul;
		for (int i = 25, mul = 1; i >= 21; i--, mul <<= 1)
			shamt += binstruction[i] * mul;

		printf("%s %d %d %d %d %d\n", opcode, rd, rs, rt, shamt, funct);
	}
	else if (type == i) {
		for (int i = 10, mul = 1; i >= 6; i--, mul <<= 1)
			rs += binstruction[i] * mul;
		for (int i = 15, mul = 1; i >= 11; i--, mul <<= 1)
			rt += binstruction[i] * mul;

		int neg = false;
		if (binstruction[16] == 1)
			neg = true;
		for (int i = 31, mul = 1; i >= 16; i--, mul <<= 1) {
			if (neg)
				value += !binstruction[i] * mul;
			else
				value += binstruction[i] * mul;
		}

		if (neg)
			value = (value * -1) - 1;

		printf("%s %d %d %d\n", opcode, rt, rs, value);
	}
	else if (type == j) {
		int neg = false;
		if (binstruction[6] == 1)
			neg = true;

		for (int i = 31, mul = 1; i >= 6; i--, mul <<= 1) {
			if (neg)
				addr += !binstruction[i] * mul;
			else
				addr += binstruction[i] * mul;
		}

		if (neg)
			addr = (addr * -1) - 1;

		printf("%s %d\n", opcode, addr);
	}

	//------------------------------

	printf("\n\n");
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
	FILE* input = fopen(filename, "r");

	if (!input) {
		printf("error!\n");
		return -EINVAL;
	}

	char command[MAX_COMMAND] = { '\0' };
	unsigned int tp_pc = pc;
	while (fgets(command, sizeof(command), input)) {
		char* tokens[MAX_NR_TOKENS] = { NULL };
		int nr_tokens = 0;

		for (size_t i = 0; i < strlen(command); i++) {
			command[i] = tolower(command[i]);
		}

		if (__parse_command(command, &nr_tokens, tokens) < 0)
			continue;

		unsigned int decimal = strtoimax(tokens[0], NULL, 16);
		int hexa[8] = { 0 };

		long div;
		int idx = 7;
		while (decimal > 0) {
			div = decimal % 16;
			decimal /= 16;

			hexa[idx--] = div;
		}

		while (idx >= 0)
			hexa[idx--] = 0;

		for (int i = 3, k = 7; i >= 0; i--, k -= 2)
			memory[tp_pc + i] = (hexa[k - 1] * 16) + hexa[k];

		tp_pc += 4;
	}

	for (int i = 3; i >= 0; i--)
		memory[tp_pc + i] = 0xff;

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
	map hm;
	makeMap(&hm);

	printf("run program -------------------------------------------------\n\n\n");
	while (true) {

		char hexa[8];
		int decimal;
		int div;

		for (int i = 0, k = 0; i < 4; i++) { //2자리씩 4번 저장된 10진수 -> 8자리 16진수
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

		unsigned int instruction = 0;
		int mul = 1;
		for (int i = 7; i >= 0; i--, mul *= 16) {
			if (hexa[i] <= 57)
				instruction += (hexa[i] - '0') * mul;
			else
				instruction += (hexa[i] - 'a' + 10) * mul;
		}

		int flag = process_instruction(instruction);
		if (!flag)  //meet halt
			break;

		pc += 4;
	}
	printf("\nfinish program -------------------------------------------------\n\n");

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
	FILE* input = fopen("C:\\Users\\dnwls\\input_file\\input2.txt","r");
	
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

	makeMap(&hm);
	makeOPlist();

	while (fgets(command, sizeof(command), input)) {
		char* tokens[MAX_NR_TOKENS] = { NULL };
		int nr_tokens = 0;

		printf("command: %s\n", command);
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