#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<stdlib.h>

#define true 1
#define false 0

#define MAX_NR_TOKENS 32	/* Maximum number of tokens in a command */
#define MAX_TOKEN_LEN 64	/* Maximum length of single token */
#define MAX_COMMAND	256		/* Maximum length of command string */


int main(void) {
	char line[MAX_COMMAND] = { '\0' };
	char* tokens[MAX_NR_TOKENS] = { NULL };
	int nr_tokens = 0;

	fgets(line, MAX_COMMAND, stdin);
	line[find_length(line) - 1] = '\0';

	parse_line(line, &nr_tokens, &tokens);

	for (int i = 0; i < nr_tokens; i++)
		printf("%s\n", tokens[i]);
}

static int find_length(char* line) {
	int len = 0;
	for (int i = 0; line[i] != '\0'; i++)
		len++;
	return len;
}

static int parse_line(char* line, int* nr_tokens, char* tokens[])
{
	char* temp;
	char token[MAX_TOKEN_LEN];
	char flag = false;
	int cur=0, token_cur=0;

	while(true) {
		if (line[cur] == '\0') { 
			temp = (char*)malloc(sizeof(char) * 20);
			for (int idx = 0; idx < token_cur; idx++)
				temp[idx] = token[idx];
			temp[token_cur] = '\0';
			tokens[(*nr_tokens)] = temp;
			(*nr_tokens)++;
			
			break;
		}

		if (line[cur] == ' ') {
			if (flag) {
				temp = (char*)malloc(sizeof(char)*20);
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
		token[token_cur++] = line[cur];

		cur++;
	}
}