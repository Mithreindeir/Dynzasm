#include "disk.h"

struct disk *disk_open(const char *file)
{
	struct disk *d = malloc(sizeof(struct disk));

	d->fd = open(file, O_RDWR | O_CREAT, 0666);

	if (d->fd == -1) {
		printf("%s\n", strerror(errno));
		printf("Error opening file\n");
		exit(1);
	}
	for (int i = 0; i < MAX_PAGES; i++)
		d->page[i] = NULL;
	d->flen = lseek(d->fd, 0, SEEK_END);

	lseek(d->fd, 0, SEEK_SET);
	d->mlen = d->flen+1;
	d->mem = malloc(d->mlen);
	read(d->fd, d->mem, d->flen);

	return d;
}

void disk_destroy(struct disk *disk)
{
	if (!disk) return;

	for (int i = 0; i < MAX_PAGES; i++) {
		free(disk->page[i]);
	}
	free(disk->mem);
	close(disk->fd);
	free(disk);
}

void *get_page(struct disk *disk, uint32_t num)
{
	if (num >= MAX_PAGES) {
		printf("Page number exceeds max of %d\n", MAX_PAGES);
		return NULL;
	}
	if (!disk->page[num]) {
		disk->page[num] = malloc(PAGE_SIZE);
		memset(disk->page[num], 0, PAGE_SIZE);
		if (lseek(disk->fd, num*PAGE_SIZE, SEEK_SET) == -1) {
			printf("Error seeking in file: %s\n", strerror(errno));
		}
		long len = ((int32_t)num+1)*PAGE_SIZE > disk->flen ? disk->flen - (num*PAGE_SIZE) : PAGE_SIZE;
		if (read(disk->fd, disk->page[num], len) < 0) {
			printf("%s\n", strerror(errno));
			printf("Error reading file\n");
			exit(1);
		}
	}
	return disk->page[num];
}

void disk_flush(struct disk *disk, int pnum, long size)
{
	if (pnum >= MAX_PAGES || !disk->page[pnum]) {
		printf("Error writing to disk\n");
		return;
	}
	if (lseek(disk->fd, pnum*PAGE_SIZE, SEEK_SET) < 0) {
		printf("Error seeking in file %s\n", strerror(errno));
		return;
	}
	if (write(disk->fd, disk->page[pnum], size) <= 0) {
		printf("Error writing to file %s\n", strerror(errno));
		return;
	}
}

void disk_write(struct disk *disk)
{
	fprintf(stderr, "Writing to disk\n");
	if (lseek(disk->fd, 0, SEEK_SET) < 0) {
		printf("Error seeking in file %s\n", strerror(errno));
		return;
	}
	if (write(disk->fd, disk->mem, disk->maxw) <= 0) {
		printf("Error writing to file %s\n", strerror(errno));
		return;
	}
}

void read_bytes(struct disk *disk, void *dst, uint32_t *offset, size_t nbytes)
{
	uint32_t off = *offset;
	if ((off+nbytes) > (uint32_t)disk->flen) {
		printf("Error: Out of bounds read\n");
		return;
	}
	memcpy(dst, (char*)disk->mem + off, nbytes);
	*offset += nbytes;
}

void write_bytes(struct disk *disk, void *src, uint32_t *offset, size_t nbytes)
{
	uint32_t off = *offset;
	if ((off+nbytes) > (uint32_t)disk->mlen) {
		disk->mlen += ((off+nbytes)-disk->mlen) + PAGE_SIZE;
		disk->mem = realloc(disk->mem, disk->mlen);
		if (!disk->mem) {
			printf("ERR\n");
			exit(1);
		}
	}
	memmove((char*)disk->mem+off, src, nbytes);
	(*offset) += nbytes;
	disk->maxw = *offset;
}
