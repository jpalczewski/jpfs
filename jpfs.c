#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filehelper.h"
#include "directory.h"



int main(int argc, char **argv)
{
	struct formatted_disk_handle *fdh;
	int result = 1;
	

	if(argc < 2)
	{
		printf("Usage: %s (create|put|get|stat|rm|destroy)", argv[0]);
		return 1;
	}
	if (strcmp(argv[1],"create")==0)
	{
		fdh = create_formatted_disk(argv[2], atoi(argv[3]), atoi(argv[4]));
		if (fdh == NULL)
		{
			puts("[FATAL] Cannot create disk!");
			printf("Usage: %s create (size) (sectors_size)", argv[0]);
			return 101;
		}
		close_formatted_disk(fdh);
		return 0;
	}
	else if (strcmp(argv[1], "destroy") == 0)
		return delete_formatted_disk(argv[2]);
	
	


	fdh = open_formatted_disk(argv[2]);
	if (fdh == NULL)
	{
		puts("[FATAL] Cannot open disk!");
		return 102;
	}

	if(strcmp(argv[1], "put") == 0)
		result = add_file(fdh, argv[3]);
	else if (strcmp(argv[1], "rm") == 0)
		result = remove_file(fdh, argv[3]);
	else if (strcmp(argv[1], "get") == 0)
		result = get_file(fdh, argv[3], argv[4]);
	else if (strcmp(argv[1], "stat") == 0)                    
	{
		list_directory(fdh);
		list_sectors(fdh->dh);
	}

	close_formatted_disk(fdh);
	return result;
}