//Day13a.c
//
//The thirteenth challenge is to 


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


int main(int argc, char **argv)
{
	long input;
	
	//No input file this time, just a number
	if (argc != 2)
	{
		fprintf(stderr, "Usage:\n\tDay13 <input filename>\n\n");
		return EXIT_FAILURE;
	}

	//A nonzero errno value tells us that strtol() failed
	errno = 0;
	input = strtol(argv[1], NULL, 10);
	if (errno != 0)
	{
		fprintf(stderr, "Error parsing input: %s\n", strerror(errno));
		fprintf(stderr, "Input should be a number!\n\n");
		return EXIT_FAILURE;
	}
	

		
	return EXIT_SUCCESS;
}
