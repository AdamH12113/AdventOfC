//Day16b.c
//
//Question 2: If we need 36,651,584 characters of data, what is the checksum
//given the provided input string?
//
//I should've seen this one coming! Using a fixed-size buffer was a bad idea.
//Fortunately, knowing how much data we need means we can determine an upper
//bound on the array size. Aside from dynamically allocating memory for the
//buffer, everything else is the same.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>


int main(int argc, char **argv)
{
	//The data array is just a pointer now
	char *data;
	size_t numChars, dataNeeded;
	int c;
	
	//No change to the command line arguments
	if (argc != 3)
	{
		fprintf(stderr, "Usage:\n\tDay16 <input data> <characters needed>\n\n");
		return EXIT_FAILURE;
	}
	
	//We need to use strtol() here since the second argument can be so big
	numChars = strlen(argv[1]);
	dataNeeded = strtol(argv[2], NULL, 10);
	
	//The algorithm always stops if we have more than dataNeeded characters. The
	//largest amount of data we can generate is thus the result of using the
	//algorithm on dataNeeded-1 characters. The output size is always 2*n + 1
	//(where n is the input size), so the max array size is 2*dataNeeded.
	data = malloc(2*dataNeeded * sizeof(char));
	if (data == NULL)
	{
		fprintf(stderr, "Error allocation memory: %s\n\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
	//Everything else is the same
	for (c = 0; c < numChars; c++)
	{
		data[c] = argv[1][c] - '0';
	}

	//No change to the dragon curve algorithm
	while (numChars < dataNeeded)
	{
		data[numChars] = 0;
		
		for (c = 0; c < numChars; c++)
		{
			data[(numChars+1) + c] = 1 - data[(numChars-1) - c];
		}

		numChars = 2*numChars + 1;
	}
	
	//Ignore any unneeded characters
	numChars = dataNeeded;
	
	//No change to the checksum
	while (numChars % 2 != 1)
	{
		for (c = 0; c < numChars; c += 2)
		{
			if (data[c] == data[c+1])
				data[c/2] = 1;
			else
				data[c/2] = 0;
		}
		
		numChars /= 2;
	}
	
	//Print the checksum, converting the data back to printable characters in
	//the process.
	printf("Checksum: ");
	for (c = 0; c < numChars; c++)
	{
		printf("%c", data[c] + '0');
	}
	printf("\n");
	
	return EXIT_SUCCESS;
}
