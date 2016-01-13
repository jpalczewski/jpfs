#include <stdio.h>



int main(int argc, char **argv)
{
	if(argc < 2)
	{
		printf("Usage: %s (create|put|get|status|destroy)", argv[0]);
		return 1;
	}

}