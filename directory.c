#include "directory.h"


struct formatted_disk_handle *create_formatted_disk(char *name, unsigned int size, unsigned int sector_size)
{
	int files;
	struct disk_handle * dh;
	struct formatted_disk_handle *fdh;
	if (size % sector_size != 0)
		return NULL;

	files = size / sector_size;
	//size = _new_size(size, sector_size);
	dh = create_disk(name, size + files*sizeof(struct file), sector_size);
	fdh = calloc(1, sizeof(struct formatted_disk_handle));
	fdh->dh = dh;
	fdh->dh->dfh->files_len = files;
	fdh->files = calloc(files, sizeof(struct file));
	
	fdh->dh->dfh->files_sector = disk_write(dh, fdh->files, sizeof(struct file)*files);

	return fdh;
}

void close_formatted_disk(struct formatted_disk_handle * fdh)
{
	disk_erase(fdh->dh, fdh->dh->dfh->files_sector);
	fdh->dh->dfh->files_sector = disk_write(fdh->dh, fdh->files, sizeof(struct file)*fdh->dh->dfh->files_len);
	close_disk(fdh->dh);
	free(fdh->files);
}

struct formatted_disk_handle* open_formatted_disk(char * name)
{
	struct formatted_disk_handle *fdh = calloc(1, sizeof(struct formatted_disk_handle));
	fdh->dh = open_disk(name);
	if (fdh->dh == NULL)
		return NULL;
	fdh->files = disk_read(fdh->dh, fdh->dh->dfh->files_sector);

	return fdh;
}

int	delete_formatted_disk(char *name)
{
	struct formatted_disk_handle *fdh;
	fdh = open_formatted_disk(name);
	if(fdh == NULL)
	{
		puts("[FATAL] Not a virtual drive!");
		return 1;
	}
	close_formatted_disk(fdh);
	return remove(name);	
}

int add_file(struct formatted_disk_handle *fdh, char *name)
{
	int available_bits = _get_free_sectors(fdh->dh)*fdh->dh->dfh->sector_size;
	int file_size, j;
	FILE *f;
	file_number selected_number;
	sector_number sector;
	char * buffer;
	char proposed_name[16];

	f = _get_file_handle_and_size(name, &file_size);

	if(f == NULL)
	{
		printf("[FATAL] Can't open %s.\n", name);
		return 1;
	}

	if(available_bits < file_size)
	{
		printf("[FATAL] Selected file is too big!");
		return 2;
	}

	selected_number = _get_file_number(fdh);
	if(selected_number == NO_PLACE_FOR_FILE)
	{
		printf("[FATAL] Unfortunately, there's no possible place in disk table to save it.");
		fclose(f);
		return 3;
	}

	
	strncpy(proposed_name, name, 16);
	proposed_name[15] = 0;

	for (j = 0; j < fdh->dh->dfh->files_len; ++j)
	{
		if(strcmp(proposed_name, fdh->files[j].name) == 0)
		{
			puts("[FATAL] File with that name exists!");
			fclose(f);
			return 4;
		}
	}


	buffer = calloc(sizeof(char), file_size);
	fread(buffer, sizeof(char), file_size, f);
	sector = disk_write(fdh->dh, buffer, file_size);
	
	fdh->files->first_sector = sector;
	memcpy(fdh->files[selected_number].name, proposed_name, 16);
	fdh->files[selected_number].name[15] = 0;
	fdh->files[selected_number].valid = 1;
	fdh->files[selected_number].size = file_size;

	free(buffer);
	fclose(f);
	return 0;
}

int get_file(struct formatted_disk_handle *fdh, char *name, char *out)
{
	file_number fn;
	FILE *f;
	char * buf;
	int result, fe;

	fn = _get_file(fdh, name);
	if (fn == NOT_FOUND)
	{
		puts("[FATAL] File not found!");
		return 1;
	}

	f = fopen(out, "wb");
	if(f == NULL)
	{
		printf("[FATAL] Can't open %s for writing!", out);
		return 2;
	}
	buf = disk_read(fdh->dh, fdh->files[fn].first_sector);
	result = fwrite(buf, sizeof(char), fdh->dh->dfh->sector_size*_file_length(fdh->dh, fdh->files->first_sector), f);
	printf("[DEBUG] get_file(): %d bits written\n", result);
	if(ferror(f))
	{
		perror("[FATAL] Can't write: ");
	}

	free(buf);
	fclose(f);

	return 0;
}

int remove_file(struct formatted_disk_handle *fdh, char *name)
{	
	file_number fn;
	
	fn = _get_file(fdh, name);
	if(fn == NOT_FOUND)
	{
		puts("[FATAL] File not found!");
		return 1;
	}

	disk_erase(fdh->dh, fdh->files[fn].first_sector);
	memset(fdh->files[fn].name, 0, 16);
	fdh->files[fn].size = 0;
	fdh->files[fn].valid = 0;

	return 0;
}

void list_directory(struct formatted_disk_handle *fdh)
{
	int i;
	int empty = 0;

	printf("Directory content:\n");
	for (i = 0; i < fdh->dh->dfh->files_len; ++i)
	{
		if(fdh->files[i].valid)
		{
			printf("\t[%d] %s - %d bits\n", fdh->files[i].first_sector, fdh->files[i].name, fdh->files[i].size);
		}
		else
		{
			empty++;
		}
	}
	printf("%d bits free(%d possible files), %d sectors available from %d.\n",
		_get_free_sectors(fdh->dh)*fdh->dh->dfh->sector_size,
		empty,
		_get_free_sectors(fdh->dh),
		fdh->dh->dfh->sector_size);
}