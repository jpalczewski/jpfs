#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filehelper.h"

int main(int argc, char **argv)
{
	/*if(argc < 2)
	{
		printf("Usage: %s (create|put|get|status|destroy)", argv[0]);
		return 1;
	}*/

	char buf[5];
	char* str = "To jest dluzszy test segmentowej inby.";
	char* str2 = "KURWA no xD Ale przypal :(";
	int j;
	struct disk_handle *dh = create_disk("test.disk", 100, 5);
	sector_number num = disk_write(dh, str, strlen(str));
	sector_number num2 = disk_write(dh, str2, strlen(str2));
	close_disk(dh);
	dh = open_disk("test.disk");
	char * ret = disk_read(dh, num);
	printf("[READ]%s\n", ret);
	char * ret2 = disk_read(dh, num2);
	printf("[READ]%s\n", ret2);
//	close_disk(dh);
	system("pause");
	return 0;
}