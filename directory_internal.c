#include "directory.h"



unsigned int _new_size(unsigned int size, unsigned int sector_size)
{
	unsigned int sectors = size / sector_size;
	//unsigned int additional_data = sectors * sizeof(struct file);
	unsigned int sectors_to_add = (sectors * sizeof(struct file)) / sector_size;
	if ((sectors * sizeof(struct file)) % sector_size)
		sectors_to_add++;
	return sectors_to_add*sector_size;
}

FILE * _get_file_handle_and_size(char *name, int * size)
{
	FILE *f;

	f = fopen(name, "r");
	if (f == NULL)
		return NULL;

	fseek(f, 0, SEEK_END);
	*size = ftell(f);
	fseek(f, 0, SEEK_SET);

	return f;
}

file_number	_get_file_number(struct formatted_disk_handle *fdh)
{
	int j;
	file_number result;

	for (j = 0; j < fdh->dh->dfh->files_len; ++j)
		if (fdh->files[j].valid == 0)
			return j;

	return NO_PLACE_FOR_FILE;
}

file_number _get_file(struct formatted_disk_handle *fdh, char *name)
{
	int j;
	char str[16];
	strncpy(str, name, 16);
	str[15] = 0;

	for (j = 0; j < fdh->dh->dfh->files_len; ++j)
	{
		if (strcmp(name, fdh->files[j].name) == 0)
		{
			return j;
		}
	}

	return NOT_FOUND;
}