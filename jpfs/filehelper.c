#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "filehelper.h"

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

jp_table * _create_jpt(int len)
{
	jp_table * new_jpt;
	int i;

	new_jpt = calloc(len, sizeof(jp_table));
	for (i = 0; i < len; ++i)
	{
		new_jpt[i] = NO_FILE;
	}

	return new_jpt;
}

struct disk_file * _create_df(unsigned int size, unsigned int sector_size)
{
	struct disk_file *ptr = calloc(1, sizeof(struct disk_file));
	DIE_ON_NULL(ptr);

	ptr->magic[0] = 'J'; ptr->magic[1] = 'P'; ptr->magic[2] = ptr->magic[3] = 0;
	ptr->size = size;
	ptr->sector_size = sector_size;
	ptr->data_offset = sizeof(struct disk_file) + (size / sector_size)*sizeof(jp_table);

	return ptr;
}

int _write_sector(struct disk_handle * disk, int sector, char * data, int size)
{
	printf("[DEBUG] _write_sector(%d, %d):%s\n", sector, size, data);
	int result;
	fseek(disk->fh, disk->dfh->data_offset + sector*disk->dfh->sector_size, SEEK_SET);
	result = fwrite(data, size, 1, disk->fh);
	
	return 0;
}

sector_number _get_first_free_sector(struct disk_handle *dh)
{
	int j;
	int size = DH_TO_LEN(dh);

	for (j = 0; j < size; ++j)
	{
		if (dh->jpth[j] == NO_FILE)
			return j;
	}
	assert(j == size);

	return -1;
}

int	_update_jpt(struct disk_handle * dh)
{
	fseek(dh->fh, sizeof(struct disk_file), SEEK_SET);
	fwrite(dh->jpth, sizeof(jp_table), DH_TO_LEN(dh), dh->fh);
	return 0;
}

sector_number disk_write(struct disk_handle *dh, char *data, unsigned int size)
{
	unsigned int j;
	unsigned int bytes_to_copy;
	char * ptr;
	sector_number first, now, prev;

	now = _get_first_free_sector(dh);
	first = now;
	for (j = 0; j < size; j+= dh->dfh->sector_size)
	{
		dh->jpth[now] = FILE_END;
		ptr = data + j;
		if ((size - j) < dh->dfh->sector_size)
			bytes_to_copy = size - j;
		else
			bytes_to_copy = dh->dfh->sector_size;
		_write_sector(dh, now, ptr, bytes_to_copy);
		if((bytes_to_copy == dh->dfh->sector_size) && (j + dh->dfh->sector_size != size))
		{
			prev = now;
			now = _get_first_free_sector(dh);
			dh->jpth[prev] = now;
		}
	}

	_update_jpt(dh);
	return first;
}

void close_disk(struct disk_handle *dh)
{
	fclose(dh->fh);
	free(dh->jpth);
	free(dh->dfh);
	/* 	free(dh);
	/* freeing the rest... */
}

struct disk_handle * create_disk(char* name, unsigned int size, unsigned int sector_size)
{
	struct disk_handle *dh;
	int	disk_len_sectors = size / sector_size;
	char * sector;

	if ((size % sector_size) != 0)
		return NULL;

	dh = calloc(1, sizeof(struct disk_handle));

#ifdef _MSC_VER
	dh->fh = fopen(name, "wb");
#else
	dh->fh = fopen(name, "w");
#endif
	DIE_ON_NULL(dh->fh);

	dh->dfh = _create_df(size, sector_size);
	DIE_ON_NULL(dh->dfh);

	dh->jpth = _create_jpt(disk_len_sectors);
	DIE_ON_NULL(dh->jpth);

	sector = calloc(sector_size, sizeof(char));

	fwrite(dh->dfh, sizeof(struct disk_file), 1, dh->fh);
	fwrite(dh->jpth, sizeof(jp_table), disk_len_sectors, dh->fh);
	

	while(--disk_len_sectors>=0)
	{
		fwrite(sector, sizeof(char), sector_size, dh->fh);
	}

	return dh;
}
struct disk_handle *open_disk(char *name)
{
	struct disk_handle *dh;
	int	disk_len_sectors;
	int result;

	dh = calloc(1, sizeof(struct disk_handle));
#ifdef _MSC_VER
	dh->fh = fopen(name, "rb+");
#else
	dh->fh = fopen(name, "r+");
#endif
	DIE_ON_NULL(dh->fh);
	
	dh->dfh = calloc(sizeof(struct disk_file), 1);
	DIE_ON_NULL(dh->dfh);
	
	result = fread(dh->dfh, sizeof(struct disk_file), 1, dh->fh);
	if (ferror(dh->fh))
		return NULL;
	
	disk_len_sectors = DH_TO_LEN(dh);
	dh->jpth = _create_jpt(disk_len_sectors);
	DIE_ON_NULL(dh->jpth);
	result = fread(dh->jpth, sizeof(jp_table), disk_len_sectors, dh->fh);
	if (ferror(dh->fh))
		return NULL;

	return dh;
}

char * _read_sector(struct disk_handle * disk, unsigned int sector)
{
	int result;
	char * buf = calloc(disk->dfh->sector_size, 1);

	fseek(disk->fh, disk->dfh->data_offset + sector*disk->dfh->sector_size, SEEK_SET);
	result = fread(buf, disk->dfh->sector_size, 1, disk->fh);
	return buf;
}


char* disk_read(struct disk_handle *dh, sector_number sn)
{
	char  *result, *newresult;
	char *newptr;
	char *ptr_new_sector;
	sector_number next = sn;
	unsigned int size = 0;

	result = calloc(size*dh->dfh->sector_size, DH_TO_LEN(dh));
	
	do {

		assert(dh->jpth[sn] != NO_FILE);
		size++;
	//	result = realloc(newresult, size*dh->dfh->sector_size);

		newptr = _read_sector(dh, sn);
		memcpy(result + (size - 1)*dh->dfh->sector_size, newptr, dh->dfh->sector_size);
		free(newptr);
		newresult = result;
	} while ((sn = dh->jpth[sn]) != FILE_END);

	return result;
}