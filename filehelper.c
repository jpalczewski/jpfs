#include "filehelper.h"

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif






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
	fseek(dh->fh, 0, SEEK_SET);
	fwrite(dh->dfh, sizeof(struct disk_file), 1, dh->fh);
	fclose(dh->fh);
	free(dh->jpth);
	free(dh->dfh);
	free(dh);
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
	dh->fh = fopen(name, "wb+");
#else
	dh->fh = fopen(name, "w+");
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

	free(sector);
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
	if ((dh->dfh->magic[0] != 'J') || (dh->dfh->magic[1] != 'P'))
		return NULL;

	disk_len_sectors = DH_TO_LEN(dh);
	dh->jpth = _create_jpt(disk_len_sectors);
	DIE_ON_NULL(dh->jpth);
	result = fread(dh->jpth, sizeof(jp_table), disk_len_sectors, dh->fh);
	if (ferror(dh->fh))
		return NULL;

	return dh;
}


char* disk_read(struct disk_handle *dh, sector_number sn)
{
	char  *result;
	char *newptr;
	sector_number next = sn;
	unsigned int size = 0;

	//result = calloc(dh->dfh->sector_size, DH_TO_LEN(dh));
	result = calloc(dh->dfh->sector_size, _file_length(dh, sn));

	do {

		assert(dh->jpth[sn] != NO_FILE);

		newptr = _read_sector(dh, sn);
		memcpy(result + (size++)*dh->dfh->sector_size, newptr, dh->dfh->sector_size);
		free(newptr);
	} while ((sn = dh->jpth[sn]) != FILE_END);

	return result;
}

int	disk_erase(struct disk_handle *dh, sector_number sn)
{
	sector_number next;
	do
	{
		assert(dh->jpth[sn] != NO_FILE);
		
		next = dh->jpth[sn];
		_unlink_sector(dh, sn);

	} while ((sn = next) != FILE_END);
}

void list_sectors(struct disk_handle *dh)
{
	int j;

	puts("Disk usage:");
	printf("[");
	for (j = 0; j < DH_TO_LEN(dh); ++j)
	{
		switch(dh->jpth[j])
		{
		case FILE_END:
			printf("%%");
			break;
		case NO_FILE:
			printf(" ");
			break;
		default:
			printf("#");
			break;
		}
	}
	
	printf("]\n");
	puts("'%' - file end, '#' - file part,' ' - free space");
}