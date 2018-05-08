#ifndef SERIALIZE_H
#define SERIALIZE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "db.h"
#include "disk.h"

struct bt_hdr {
	int32_t info;
	int32_t knum;
	int32_t next_page;
};

struct bt_key {
	uint32_t roff;
	uint32_t ksize;
	char * key;
};

struct db_node *deserialize(struct disk *disk);
void serialize(struct db_node *root, struct disk *disk);

#endif
