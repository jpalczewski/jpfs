#include "filehelper.h"

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

int	_update_jpt(struct disk_handle * dh)
{
	fseek(dh->fh, sizeof(struct disk_file), SEEK_SET);
	fwrite(dh->jpth, sizeof(jp_table), DH_TO_LEN(dh), dh->fh);
	return 0;
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
	//printf("[DEBUG] _write_sector(%d, %d):%s\n", sector, size, data);
	int result;
	fseek(disk->fh, disk->dfh->data_offset + sector*disk->dfh->sector_size, SEEK_SET);
	result = fwrite(data, size, 1, disk->fh);

	return 0;
}

char * _read_sector(struct disk_handle * disk, unsigned int sector)
{
	int result;
	char * buf = calloc(disk->dfh->sector_size, 1);

	fseek(disk->fh, disk->dfh->data_offset + sector*disk->dfh->sector_size, SEEK_SET);
	result = fread(buf, disk->dfh->sector_size, 1, disk->fh);
	return buf;
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
	return NO_SPACE;
}

sector_number _get_free_sectors(struct disk_handle *dh)
{
	int j;
	sector_number sum = 0;
	int size = DH_TO_LEN(dh);

	for (j = 0; j < size; ++j)
	{
		if (dh->jpth[j] == NO_FILE)
			sum++;
	}

	return sum;
}

int	_unlink_sector(struct disk_handle *dh, sector_number which)
{
	char *empty;

	dh->jpth[which] = NO_FILE;
	empty = calloc(dh->dfh->sector_size, sizeof(char));
	fseek(dh->fh, dh->dfh->data_offset + dh->dfh->sector_size*which, SEEK_SET);
	fwrite(empty, sizeof(char), dh->dfh->sector_size, dh->fh);

	return 0;
}

int _file_length(struct disk_handle *dh, sector_number which)
{
	int size = 1;
	
	while(which != FILE_END)
	{
		assert(which != NO_FILE);
		size++;
		which = dh->jpth[which];
	}

	return size;
}