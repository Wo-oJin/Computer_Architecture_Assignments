/**********************************************************************
 * Copyright (c) 2021
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
/* To avoid security error on Visual Studio */
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#define MAX_NR_TOKENS 32	/* Maximum number of tokens in a command */
#define MAX_TOKEN_LEN 64	/* Maximum length of single token */
#define MAX_COMMAND	256		/* Maximum length of command string */

#define true 1
#define false 0

/***********************************************************************
 * parse_command
 *
 * DESCRIPTION
 *	parse @command, and put each command token into @tokens[] and the number of
 *	tokes into @nr_tokens.
 *
 *  A command token is defined as a string without any whitespace (i.e., *space*
 *  and *tab* in this programming assignment). For exmaple,
 *    command = "  Hello world   Ajou   University!!  "
 *
 *  then, nr_tokens = 4 and tokens should be
 *   tokens[0] = "Hello"
 *   tokens[1] = "workd"
 *   tokens[2] = "Ajou"
 *   tokens[3] = "University!!"
 *
 *  Another exmaple is;
 *   command = "  add r0   r1 r2 "
 *
 *  then, nr_tokens = 4, and tokens are
 *   tokens[0] = "add"
 *   tokens[1] = "r0"
 *   tokens[2] = "r1"
 *   tokens[3] = "r2"
 *
 *
 * RESTRICTION
 *  DO NOT USE strtok or equivalent libraries. You should implement the
 *  feature by your own to get the points.
 *
 *
 * RETURN VALUE
 *	Return 0 after filling in @nr_tokens and @tokens[] properly
 *
 */

static int parse_command(char* command, int* nr_tokens, char* tokens[])
{
	char* temp; // parsing�� token�� tokens �迭�� �ű�� ���� �Ź� �����Ҵ� �Ǵ� �ӽ� �迭
	char token[MAX_TOKEN_LEN]; // parsing ����� �Ǵ� token�� �����ϴ� �迭
	char flag = false; // white space�� ������ �� ���� ����Ǿ� �ִ� token�� ������ �Ǻ��ϴ� ����
	int cur = 0, token_cur = 0; // ���� command�� index ��ġ, ���� token�� index ��ġ

	while (true) {
		if (command[cur] == '\0') {
			if (flag) {
				temp = (char*)malloc(sizeof(char) * MAX_TOKEN_LEN);
				for (int j = 0; j < token_cur; j++)
					temp[j] = token[j];
				temp[token_cur] = '\0';
				tokens[(*nr_tokens)] = temp;
				(*nr_tokens)++;
				token_cur = 0;
			}

			break;
		}

		if (isspace(command[cur]) != 0) {
			if (flag) {
				temp = (char*)malloc(sizeof(char) * MAX_TOKEN_LEN);
				for (int j = 0; j < token_cur; j++)
					temp[j] = token[j];
				temp[token_cur] = '\0';
				tokens[(*nr_tokens)] = temp;
				(*nr_tokens)++;
				token_cur = 0;
			}
			flag = false;
			cur++;
			continue;
		}

		flag = true;
		token[token_cur++] = command[cur];

		cur++;
	}
}

/***********************************************************************
 * The main function of this program. DO NOT CHANGE THE CODE BELOW
 */
int main(int argc, const char* argv[])
{
	char line[MAX_COMMAND] = { '\0' };
	FILE* input = stdin;

	if (argc == 2) {
		input = fopen(argv[1], "r");
		if (!input) {
			fprintf(stderr, "No input file %s\n", argv[1]);
			return -EINVAL;
		}
	}

	while (fgets(line, sizeof(line), input)) {
		char* tokens[MAX_NR_TOKENS] = { NULL };
		int nr_tokens = 0;

		parse_command(line, &nr_tokens, tokens);

		fprintf(stderr, "nr_tokens = %d\n", nr_tokens);
		for (int i = 0; i < nr_tokens; i++) {
			fprintf(stderr, "tokens[%d] = %s\n", i, tokens[i]);
		}
		printf("\n");
	}

	if (input != stdin) fclose(input);

	return 0;
}