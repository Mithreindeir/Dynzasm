#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "arch/x86/x86.h"
#include "arch/x86/x86load.h"
#include "common/trie.h"
#include "common/db.h"
#include "common/disk.h"
#include "common/ser.h"
#include "disas.h"

int main()
{
	struct disk *d = disk_open("ex.db");
	struct db_node *rdb = deserialize(d);
	struct db_key ink;
	char * args[] = { "0x4f", "foo"};
	ink.key = args[0];
	ink.ksize = strlen(args[0]);
	ink.psize = strlen(args[1])+1;
	ink.ptr = malloc(ink.psize);
	memcpy(ink.ptr, args[1], ink.psize-1);
	((char*)ink.ptr)[ink.psize-1] = 0;
	rdb = db_insert(rdb, ink);
	struct disassembler *ds = ds_init(X86_ARCH, MODE_32B);

	unsigned char bytes[] = "\x55\x89\xe5\x83\xec\x10\xe8\x44\x00\x00\x00\x05\xee\x1a\x00\x00\xc7\x45\xf8\x00\x00\x00\x00\xc7\x45\xfc\x00\x00\x00\x00\xeb\x22\x8b\x45\xfc\x2b\x45\xf8\x8b\x4d\xf8\x8b\x55\xfc\x01\xca\x50\xff\x75\xfc\x52\xe8\xae\xff\xff\xff\x83\xc4\x0c\x01\x45\xf8\x83\x45\xfc\x01\x83\x7d\xfc\x09\x7e\xd8\xb8\x00\x00\x00\x00\xc9\xc3";
	//unsigned char bytes[] = "\x55\x48\x89\xe5\x48\x83\xec\x10\xc7\x45\xf8\x00\x00\x00\x00\xc7\x45\xfc\x00\x00\x00\x00\xeb\x23\x8b\x45\xfc\x2b\x45\xf8\x89\xc2\x8b\x4d\xf8\x8b\x45\xfc\x01\xc1\x8b\x45\xfc\x89\xc6\x89\xcf\xe8\xb2\xff\xff\xff\x01\x45\xf8\x83\x45\xfc\x01\x83\x7d\xfc\x09\x7e\xd7\xb8\x00\x00\x00\x00\x5d\xc3";

	ds_decode(ds, bytes, sizeof(bytes)-1, 0x0);
	struct dis *dis = NULL;
	DS_FOREACH(ds, dis) {
		int iter = 0;
		for (int i = 0; i < dis->num_operands; i++) {
			iter += operand_squash_replace(dis->op_squash+iter, SQUASH_SIZE-iter, dis->operands[i],rdb);
			if ((i+1) < dis->num_operands)
				iter += snprintf(dis->op_squash+iter, SQUASH_SIZE-iter, ", ");
		}
		printf("%#08lx:\t%s\t%s\n", dis->address, dis->mnemonic, dis->op_squash);
	}

	ds_destroy(ds);
	disk_destroy(d);
	return 0;
}
