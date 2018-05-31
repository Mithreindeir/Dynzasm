#include "lex.h"

char **lex(char *string, char *delim, int *num_tokens, int type)
{
	char **tokens = NULL;
	int nt = 0, ptype = -1;
	char *iter = string, *max = string + strlen(string);
	while (iter < max) {
		char * token = iter;
		int ctype=0, clen=0, tlen = 0;
		while (token < max && !strchr(delim, *iter) && !(ctype=token_type(iter, &clen, type,ptype))) {
			tlen += clen, iter += clen;
			ptype = ctype;
		}
		if (ctype && !tlen) {
			tlen += clen;
		} else if (!ctype && !tlen) {
			iter++;
			continue;
		}
		nt++;
		if (!tokens) tokens = malloc(sizeof(char*));
		else tokens = realloc(tokens, sizeof(char*)*nt);

		char *atok = malloc(tlen+1);
		memcpy(atok, token, tlen);
		atok[tlen] = 0;
		tokens[nt-1] = atok;
		iter = token + (tlen?tlen:1);
		ptype = ctype;
	}
	*num_tokens = nt;
	return tokens;
}

int token_type(char *str, int *len, int type, int ptype)
{
	assert(type < (int)(sizeof(symbol_tt)/sizeof(char **)));
	const char ** symbols = SYMTS(type);
	const int * symbols_type = SYMTT(type);
	for (int i=0; symbols[i] && symbols_type[i]; i++) {
		int slen = strlen(symbols[i]);
		if (!strncmp(str, symbols[i], slen)) {
			*len = slen;
			return symbols_type[i];
		}
	}

	if (ptype != t_notype && !strncmp(str, "0x", 2) && isxdigit(str[2])) {
		int l = 2;
		while (isxdigit(str[++l]));
		*len = l;
		return t_number;
	} else if (ptype != t_notype && isdigit(str[0])) {
		int l = 1;
		while (isdigit(str[l])) l++;
		*len = l;
		return t_number;
	}

	*len = 1;
	return t_notype;
}
