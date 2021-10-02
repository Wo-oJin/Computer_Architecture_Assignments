#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#define true 1
#define false 0

#define MAP_SIZE 32
#define WORD_LEN 5

enum slotStatus { empty, deleted, inuse};
typedef int (*hashfunc)(int key);

typedef struct Slot {
	char key[WORD_LEN];
	int* val;
	enum slotStatus status;
}slot;

typedef struct HashMap {
	slot bucket[MAP_SIZE];
	hashfunc hf;
}map;

void mapInit(map* hm, hashfunc hf) {
	for (int i = 0; i < MAP_SIZE; i++)
		(hm->bucket[i]).status = empty;
	hm->hf = hf;
}

int makeHash(char* str) {
	int hash=0;
	while(*str!='\0') {
		hash += (int)(*str)%MAP_SIZE;
		str++;
	}

	return hash % MAP_SIZE;
}

void insert(map* hm, char* key, int val1, int val2) {
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

int* search(map* hm, char* key) {
	int hv = hm->hf(key);
	int temp = hv;
	while (strcmp(key, hm->bucket[hv].key)) {
		hv++;
		hv %= MAP_SIZE;

		if (hv == temp)
			return -1;
	}

	return hm->bucket[hv].val;
}

void makeMap(map* hm) {
	mapInit(hm, makeHash);
	int* arr;

	//add sub and or nor
	insert(hm, "add", 0, 0x20); insert(hm, "t0", 8, 0);
	insert(hm, "t1", 9, 0); insert(hm, "t2", 10, 0);

	insert(hm, "s1", 17, 0); insert(hm, "s3", 19, 0);

	insert(hm, "sub", 0, 0x22); insert(hm, "t3", 11, 0);
	insert(hm, "t4", 12, 0); insert(hm, "t5", 13, 0);

	insert(hm, "and", 0, 0x24); insert(hm, "s0", 16, 0);
	insert(hm, "a0", 4, 0); insert(hm, "a2", 6, 0);

	insert(hm, "or", 0, 0x25); insert(hm, "s2",18, 0);
	insert(hm, "zero", 0, 0); 

	insert(hm, "nor", 0, 0x27); insert(hm, "t9", 25, 0);
	insert(hm, "sp", 29, 0); insert(hm, "gp", 28, 0);
}

static int parse_command(char* assembly, int* nr_tokens, char* tokens[])
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

void ten_to_two(int* instruction, int n, int size, int* idx) {
	int* arr = (int*)malloc(sizeof(int) * size);
	int tp = size;
	size -= 1;

	while (n >= 1) {
		arr[size--] = n % 2;
		n >>= 2;
	}

	while (size >= 0) 
		arr[size--] = 0;

	for (int k=0; k<tp; k++) 
		instruction[(*idx)++] = arr[k];
}


char ten_to_six(int n) {
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


void makeRformat(map* hm, int* r_format,char* assembly) {
	int nr_tokens;
	char** tokens = (char**)malloc(sizeof(char*) * 5);
	parse_command(assembly, &nr_tokens, tokens);

	int* op_funct = search(hm, tokens[0]);
	r_format[0] = op_funct[0]; 
	r_format[5] = op_funct[1];

	int* val = search(hm, tokens[1]);
	r_format[3] = val[0];

	val = search(hm, tokens[2]);
	r_format[1] = val[0];

	val = search(hm, tokens[3]);
	r_format[2] = val[0];

	r_format[4] = 0;
}

int main(void) {
	map hm;
	makeMap(&hm);
	int r_format[6];

	char assembly[] = "nor t9 sp gp";
	makeRformat(&hm, &r_format, assembly); //어셈블러 -> R_format으로 변환

	for (int i = 0; i < 6; i++)
		printf("%d ", r_format[i]);
	printf("\n");

	int idx = 0;
	int instruction[32];
	int size[6] = { 6,5,5,5,5,6 };

	for (int i = 0; i < 6; i++) { //R_format의 각 필드값을 2진수로 전환
		ten_to_two(&instruction, r_format[i], size[i], &idx);
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