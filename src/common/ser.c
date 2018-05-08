#include "ser.h"

struct db_node *deserialize(struct disk *disk)
{
	if (disk->flen <= 1) return db_node_init(NULL);
	uint32_t diter = 0;
	struct bt_hdr hdr;
	read_bytes(disk, &hdr, &diter, sizeof(struct bt_hdr));

	int knum = hdr.knum;
	//printf("Num Keys: %d\n", knum);
	struct db_node *rdb = db_node_init(NULL);
	for (int i = 0; i < knum; i++) {
		struct bt_key key;
		read_bytes(disk, &key.roff, &diter, sizeof(uint32_t));
		read_bytes(disk, &key.ksize, &diter, sizeof(uint32_t));
		key.key = malloc(key.ksize);
		read_bytes(disk, key.key, &diter, key.ksize);
		//printf("key: %.*s size: %d\n", key.ksize, key.key, key.ksize);
		struct db_key k;
		k.key = key.key;
		k.ksize = key.ksize;
		k.ptr = NULL;
		k.psize = 0;
		uint32_t tpsize = 0;
		read_bytes(disk, &tpsize, &key.roff, sizeof(uint32_t));
		k.psize = tpsize;
		k.ptr = malloc(k.psize);
		read_bytes(disk, k.ptr, &key.roff, k.psize);
		rdb = db_insert(rdb, k);
		free(key.key);
	}
	return rdb;
}

void serialize(struct db_node *root, struct disk *disk)
{
	//printf("Serializing:\n");
	fprintf(stderr, "Serializing:\n");
	struct db_node *leaf = NULL;
	int len = db_getleaves(root, &leaf);

	uint32_t off = 0;
	struct bt_hdr hdr;
	hdr.knum = len;
	hdr.info = 0;
	hdr.next_page = 0;
	write_bytes(disk, &hdr, &off, sizeof(struct bt_hdr));

	int used_mem = 0;;
	struct db_node *sl = leaf;
	/*Dry run calculate used memory*/
	while (leaf) {
		for (int i = 0; i < leaf->num_keys; i++) {
			used_mem += 2 * sizeof(uint32_t);
			used_mem += leaf->keys[i].ksize;
		}
		leaf = leaf->next;
	}
	leaf = sl;
	uint32_t diter = used_mem + off;
	while (leaf) {
		for (int i = 0; i < leaf->num_keys; i++) {
			uint32_t ksize = leaf->keys[i].ksize;
			uint32_t roff = diter;
			//printf("key: %.*s size: %d\n", ksize, (char*)leaf->keys[i].key, ksize);
			write_bytes(disk, &roff, &off, sizeof(uint32_t));
			write_bytes(disk, &ksize, &off, sizeof(uint32_t));
			write_bytes(disk, leaf->keys[i].key, &off, ksize);

			write_bytes(disk, &leaf->keys[i].psize, &diter, sizeof(uint32_t));
			//printf("ptr: %.*s %d\n", leaf->keys[i].ksize, leaf->keys[i].key, leaf->keys[i].psize);
			write_bytes(disk, leaf->keys[i].ptr, &diter, leaf->keys[i].psize);
		}


		leaf = leaf->next;
	}
}
