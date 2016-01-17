#ifndef DIRECTORY_H_
#define DIRECTORY_H_
#include "filehelper.h"

struct file
{
	char name[16];
	sector_number first_sector;
	int size;
	unsigned int valid : 1;
};


typedef int file_number;
#define NO_PLACE_FOR_FILE -1
#define NOT_FOUND -2

struct formatted_disk_handle
{
	struct disk_handle *dh;
	struct file * files;
};

unsigned int	_new_size(unsigned int size, unsigned int sector_size);
FILE *			_get_file_handle_and_size(char *name, int * size);
file_number		_get_file_number(struct formatted_disk_handle *fdh);
file_number		_get_file(struct formatted_disk_handle *fdh, char *name);

struct formatted_disk_handle	*create_formatted_disk(char *name, unsigned int size, unsigned int sector_size);
void							close_formatted_disk(struct formatted_disk_handle * fdh);
struct formatted_disk_handle*	open_formatted_disk(char * name);
int							delete_formatted_disk(char * name);

int add_file(struct formatted_disk_handle *fdh, char *name);
int get_file(struct formatted_disk_handle *fdh, char *name, char *out);
int remove_file(struct formatted_disk_handle *fdh, char *name);
void list_directory(struct formatted_disk_handle *fdh);
#endif