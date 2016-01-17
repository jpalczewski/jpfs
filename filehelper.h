#ifndef FILEHELPER_H_	
#define FILEHELPER_H_
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#define FILE_END	-1 
#define NO_FILE		-2
#define NO_SPACE	-3
typedef int sector_number;
typedef int jp_table;


#define DH_TO_LEN(dh) (dh->dfh->size / dh->dfh->sector_size)			
#define DIE_ON_NULL(x) if((x)==NULL) return NULL;

struct disk_file
{
	char magic[4];
	unsigned int size;
	unsigned int sector_size;
	unsigned int files_len;
	unsigned int files_sector;
	unsigned int data_offset;
};

struct disk_handle
{
	FILE *fh;
	struct disk_file *dfh;
	jp_table *jpth;
};


jp_table			*_create_jpt(int len);
int					_update_jpt(struct disk_handle * dh);
struct disk_file	*_create_df(unsigned int size, unsigned int sector_size);
int					_write_sector(struct disk_handle * disk, int sector, char * data, int size);
char				*_read_sector(struct disk_handle *disk, unsigned int sector);
sector_number		_get_first_free_sector(struct disk_handle *dh);
sector_number _get_free_sectors(struct disk_handle *dh);
int					_unlink_sector(struct disk_handle *dh, sector_number which);
int				_file_length(struct disk_handle *dh, sector_number which);

struct disk_handle	*create_disk(char* name, unsigned int size, unsigned int sector_size);
struct disk_handle	*open_disk(char *name);
void				close_disk(struct disk_handle *dh);

sector_number		disk_write(struct disk_handle *dh, char *data, unsigned int size);
char*				disk_read(struct disk_handle *dh, sector_number sn);
int					disk_erase(struct disk_handle *dh, sector_number sn);

void				list_sectors(struct disk_handle *dh);
#endif