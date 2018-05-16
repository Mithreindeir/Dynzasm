#ifndef DISK_H
#define DISK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>

#define PAGE_SIZE 4096
#define MAX_PAGES 100
/* Buffer Manager for IO
 * I haven't made B+ Trees in pages yet so this module is mostly irrelevant */

struct disk {
	int fd, flen;
	void *page[MAX_PAGES];
	/*Temporarily just use buffer for entire file*/
	void *mem;
	int mlen, maxw;
};

struct disk *disk_open(const char *file);
void disk_destroy(struct disk *disk);

void *get_page(struct disk *disk, uint32_t num);
void disk_flush(struct disk *disk, int pnum, long size);
void disk_write(struct disk *disk);

void read_bytes(struct disk *disk, void *dst, uint32_t *offset, size_t bytes);
void write_bytes(struct disk *disk, void *src, uint32_t *offset, size_t bytes);

#endif
