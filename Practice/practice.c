#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#define true 1
#define false 0

#define MAP_SIZE 50
#define WORD_LEN 5

enum slotStatus { empty, deleted, inuse};
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

void mapInit(map* hm, hashfunc hf) {
	for (int i = 0; i < MAP_SIZE; i++)
		(hm->bucket[i]).status = empty;
	hm->hf = hf;
}

int makeHash(char* str) { //인자로 전달된 key값을 hash value로 전환
	int hash=0;
	while(*str!='\0') {
		hash += (int)(*str)%MAP_SIZE;
		str++;
	}

	return hash % MAP_SIZE;
}

void insert(map* hm, char* key, int val1, int val2) { //Hash Map에 인자로 전달된 (key, value)를 저장
	int* arr = (int*)malloc(sizeof(int) * 2);
	arr[0] = val1; arr[1] = val2;
	int hv = hm->hf(key);

	while (hm->bucket[hv].status == inuse) {
		hv++;
		hv %= MAP_SIZE;
	}

	hm->bucket[hv].val=arr;
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
	insert(hm, "add", 0, 0x20); insert(hm, "addi", 0x08, 0); insert(hm, "andi", 0x0c, 0); insert(hm, "or", 0, 0x25);
	insert(hm, "sub", 0, 0x22); insert(hm, "and", 0, 0x24); insert(hm, "ori", 0x0d, 0); insert(hm, "nor", 0, 0x27);
	insert(hm, "sll", 0, 0x00); insert(hm, "srl", 0, 0x02); insert(hm, "sra", 0, 0x03); insert(hm, "lw", 0x23,0);
	insert(hm, "sw", 0x2b,0); 

	// register number
	insert(hm, "zero", 0, 0); insert(hm, "at", 1, 0); insert(hm, "v0", 2, 0); insert(hm, "v1", 3, 0);
	insert(hm, "a0", 4, 0); insert(hm, "a1", 5, 0); insert(hm, "a2", 6, 0); insert(hm, "a3", 7, 0);
	insert(hm, "t0", 8, 0); insert(hm, "t1", 9, 0); insert(hm, "t2", 10, 0); insert(hm, "t3", 11, 0);
	insert(hm, "t4", 12, 0); insert(hm, "t5", 13, 0); insert(hm, "t6", 14, 0); insert(hm, "t7", 15, 0);
	insert(hm, "s0", 16, 0); insert(hm, "s1", 17, 0); insert(hm, "s2", 18, 0); insert(hm, "s3", 19, 0);
	insert(hm, "s4", 20, 0); insert(hm, "s5", 21, 0); insert(hm, "s6", 22, 0); insert(hm, "s7", 23, 0);
	insert(hm, "t8", 24, 0); insert(hm, "t9", 25, 0); insert(hm, "k1", 26, 0); insert(hm, "k2", 27, 0);
	insert(hm, "gp", 28, 0); insert(hm, "sp", 29, 0); insert(hm, "fp", 30, 0); insert(hm, "ra", 31, 0); 
}

static int parse_command(char* assembly, int* nr_tokens, char* tokens[]) //사용자가 입력한 어셈블리어를 white space를 기준으로 split
{
	char* curr = assembly;
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

	return 0;
}

void ten_to_two(int* instruction, int n, int size, int* idx) { //전달된 크기만큼의 인덱스를 갖는 10진수 -> 2진수 변환
	int* arr = (int*)malloc(sizeof(int) * size);
	int tp = size;
	char negative = false;
	size -= 1;

	if (n < 0) {
		n *= -1;
		negative = true;
	}

	while (n >= 1) {
		arr[size--] = n % 2;
		n >>= 1;
	}

	while (size >= 0) 
		arr[size--] = 0;

	for (int k = 0; k < tp; k++) {
		if(negative)
			instruction[(*idx)++] = !arr[k];
		else
			instruction[(*idx)++] = arr[k];
	}
}

char ten_to_six(int n) { //0~15 범위의 10진수 -> 16진수로 변환
	switch (n) {
	case 10:
		return 'a';
	case 11:
		return 'b';
	case 12:
		return 'c';
	case 13:
		return 'd';
	case 14:
		return 'e';
	case 15:
		return 'f';
	default:
		return n+'0';
	}
}

int strtonum(char* num) { //I_format, shift 어셈블리어에 포함된 10진수, 16진수 string 타입의 상수를 알맞은 정수로 반환 
	char* ptr;

	if (num[0] == '-') {
		if (num[1] == '0')
			return strtoimax(num, &ptr, 16) + 1;
		else
			return strtoimax(num, &ptr, 10) + 1;
	}
	else {
		if (num[0] == '0')
			return strtoimax(num, &ptr, 16);
		else
			return strtoimax(num, &ptr, 10);
	}
}

// op(6) rs(5) rt(5) rd(5) shamt(5) funct(6)
void makeRformat(map* hm, int* r_format, char* tokens[], int shamt) { //어셈블리어 -> R_format 변환
	char flag = false;
	if (shamt > 0)
		flag = true;
	int* op_funct = search(hm, tokens[0]);
	r_format[0] = op_funct[0]; 

	int* val = search(hm, tokens[2]);
	if (!flag) 
		r_format[1] = val[0];
	else
		r_format[2] = val[0];

	val = search(hm, tokens[3]);
	if(!flag)
		r_format[2] = val[0];
	else
		r_format[1] = 0;
	
	val = search(hm, tokens[1]);
	r_format[3] = val[0];

	r_format[4] = shamt;
	r_format[5] = op_funct[1];
}

// op(6) rs(5) rt(5) constant or address(16) 
void makeIformat(map* hm, int* I_format, char* tokens[]) { //어셈블리어 -> I_format 변환
	int* op_funct = search(hm, tokens[0]);
	I_format[0] = op_funct[0];

	int* val = search(hm, tokens[2]);
	I_format[1] = val[0];

	val = search(hm, tokens[1]);
	I_format[2] = val[0];

	I_format[3] = strtonum(tokens[3]);
}

int main(void) {
	map hm;
	makeMap(&hm);
	int* format;
	int format_size;
	char flag = false;
	char assembly[] = "addi sp sp -0x10";

	int nr_tokens;
	char** tokens = (char**)malloc(sizeof(char*) * 5);
	parse_command(assembly, &nr_tokens, tokens);

	if (search(&hm, tokens[3]) == NULL) {
		if (!strcmp("sll", tokens[0]) || !strcmp("srl", tokens[0])) {
			format_size = 6;
			format = (int*)malloc(sizeof(int) * format_size);
			makeRformat(&hm, format, tokens,strtonum(tokens[3])); //어셈블리어 -> R_format으로 변환
		}
		else {
			flag = true;
			format_size = 4;
			format = (int*)malloc(sizeof(int) * format_size);
			makeIformat(&hm, format, tokens); //어셈블리어 -> I_format으로 변환
		}
	}
	else {
		format_size = 6;
		format = (int*)malloc(sizeof(int) * format_size);
		makeRformat(&hm, format, tokens, 0); //어셈블리어 -> R_format으로 변환
	}
	for (int i = 0; i < format_size; i++)
		printf("%d ", format[i]);
	printf("\n");

	int idx = 0;
	int instruction[32];
	int R_size[6] = { 6,5,5,5,5,6 };
	int I_size[4] = { 6,5,5,16 };

	if (!flag) {
		for (int i = 0; i < format_size; i++) { //R_format의 각 필드값을 2진수로 전환
			ten_to_two(instruction, format[i], R_size[i], &idx);
		}
	}
	else {
		for (int i = 0; i < format_size; i++) { //I_format의 각 필드값을 2진수로 전환
			ten_to_two(instruction, format[i], I_size[i], &idx);
		}
	}

	for (int i = 0; i < 32; i++)
		printf("%d", instruction[i]);
	printf("\n");

	char OXinstruction[8];
	int num = 0, a=8;

	for (int i = 0, k = 0; i <= 32;) { //32자리의 2진수를 4자리씩 끊어서 8개의 16진수를 만듬
		num += instruction[i] * a;
		a >>= 1;
		i++;
		if (i % 4 == 0) {
			a = 8;
			OXinstruction[k++] = ten_to_six(num);
			num = 0;
		}
	}

	for (int i = 0; i < 8; i++)
		printf("%c ", OXinstruction[i]);

	printf("\n");

	char* ptr;
	int n = strtoimax(OXinstruction, &ptr, 16); //문자열 형태의 instruction을 16진수로 int로 변환
	printf("0x%08x\n", n);
}