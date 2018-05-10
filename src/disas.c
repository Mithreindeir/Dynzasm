#include "disas.h"

struct disassembler *ds_init(int arch)
{
	struct disassembler *ds = malloc(sizeof(struct disassembler));

	ds->arch = arch;
	ds->root = NULL;
	ds->instr = NULL, ds->num_instr = 0;
	if (arch == X64_ARCH) {
		ds->root = trie_init(0, NULL);
		x86_parse(ds->root);
	}

	return ds;
}

void ds_destroy(struct disassembler *ds)
{
	if (!ds) return;

	for (int i = 0; i < ds->num_instr; i++) {
		dis_destroy(ds->instr[i]);
	}

	trie_destroy(ds->root);
	free(ds->instr);
	free(ds);
}

void ds_decode(struct disassembler *ds, unsigned char *stream, int size, uint64_t entry)
{
	if (ds->arch == X64_ARCH) {
		int iter = 0;
		int addr = entry;
		while (iter < size) {
			int used_bytes = 0;
			struct dis *disas = x86_disassemble(ds->root, stream+iter, size-iter, addr, &used_bytes);
			iter += used_bytes;
			addr += used_bytes;
			if (!disas) continue;

			disas->address = addr-used_bytes;
			dis_squash(disas);
			ds->num_instr++;
			if (!ds->instr)
				ds->instr = malloc(sizeof(struct dis*));
			else
				ds->instr = realloc(ds->instr, sizeof(struct dis*)*ds->num_instr);
			ds->instr[ds->num_instr-1] = disas;
		}
	}
}
