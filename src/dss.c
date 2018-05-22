#include "dss.h"

struct dsem *parse_semantic(char **buffer)
{
	char *buf = *buffer;
	char *line = strtok(buf, "\n");
	if (!line) return NULL;
	struct dsem *sem = NULL;
	do {
		if (!sem) {/*If semantic is NULL, try and the starting line of "[mnem] grp-id"*/
			char *dstr = strchr(buf, ']');
			if (!dstr) continue;
			dstr[0] = 0;
			dstr++;
			char *mstr = strchr(line, '[')+1;
			if (!mstr) continue;
			int v = strtol(dstr, NULL, 10);
			sem = dsem_init(mstr, v);
		/*Otherwise try and read the read/write flag/operand lines*/
		} else if (!strncmp(line, "o:", 2)) {
			parse_rwoperands(sem, line+2);
		} else if (!strncmp(line, "f:", 2)) {
			parse_rwflags(sem, line+2);
		}
	} while ((line=strtok(NULL, "\n")) && !!strncmp(line, "end", 3));

	*buffer = strtok(NULL, "\0");
	return sem;
}

void parse_rwoperands(struct dsem *sem, char *line)
{
	int len = strlen(line);
	char *val = line;
	while (val!=(line+len)) {
		char *rw = strchr(val, '$');
		if (!rw) break;
		rw++;
		val = strchr(val, '=');
		if (!val) break;
		val++;
		val = strchr(val, '$');
		if (!val) break;
		val++;
		int amask = 0;
		while (*rw=='r'||*rw=='w') {
			amask |= *rw=='r'?DSEM_READ:DSEM_WRITE;
			rw++;
		}
		dsem_add(sem, strtol(val, NULL, 0), amask);
	}
}

void parse_rwflags(struct dsem *sem, char *line)
{
	int len = strlen(line);
	char *val = line;
	while (val && val!=(line+len) && *val) {
		char *ft = strchr(val, '$');
		if (!ft) return;
		val = strchr(val, '=');
		if (!val) return;
		val++;
		unsigned char flags = 0;
		int fv = 0;
		while ((fv=val[0])) {
			flags |= FVAL(fv);
			val++;
		}
		if (!strncmp(ft, "$mf", 3))
			sem->mflags |= flags;
		if (!strncmp(ft, "$rf", 3))
			sem->rflags |= flags;
	}
}

struct dsem *dsem_init(char *mnemonic, int group)
{
	struct dsem *sem = malloc(sizeof(struct dsem));
	sem->mnemonic = mnemonic, sem->group = group, sem->rflags = 0, sem->mflags = 0;
	sem->read = NULL, sem->nread = 0;
	sem->write = NULL, sem->nwrite = 0;
	return sem;
}

void dsem_destroy(struct dsem *sem)
{
	if (!sem) return;
	free(sem->read);
	free(sem->write);
	free(sem);
}

void dsem_add(struct dsem *sem, int val, int rw)
{
	if (rw & DSEM_READ) {
		sem->nread++;
		if (!sem->read)
			sem->read=malloc(sizeof(int));
		else
			sem->read=realloc(sem->read, sizeof(int)*sem->nread);
		sem->read[sem->nread-1] = val;
	}
	if (rw & DSEM_WRITE) {
		sem->nwrite++;
		if (!sem->write)
			sem->write=malloc(sizeof(int));
		else
			sem->write=realloc(sem->write, sizeof(int)*sem->nwrite);
		sem->write[sem->nwrite-1] = val;
	}
}

void dsem_print(struct dsem *sem)
{
	if (!sem) return;
	printf("MNEMONIC: %s\tGROUP: %d\n", sem->mnemonic, sem->group);
	printf("READ: ");
	for (int i = 0; i < sem->nread; i++) {
		printf("opr_%d%c", sem->read[i], (i+1)==sem->nread?'\n':',');
	}
	printf("WRITE: ");
	for (int i = 0; i < sem->nwrite; i++) {
		printf("opr_%d%c", sem->write[i], (i+1)==sem->nwrite?'\n':',');
	}
	printf("Modified Flags: ");
	for (int i = 7; i >= 0; i--) {
		int val = sem->mflags & (1 << i);
		printf("%c", VALF(val));
	}
	printf("\n");
	printf("Read Flags: ");
	for (int i = 7; i >= 0; i--) {
		int val = sem->rflags & (1 << i);
		printf("%c", VALF(val));
	}
	printf("\n");
}
