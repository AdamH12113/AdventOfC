//Day6b.c
//
//Question 2: If the message is made up of the least common letters instead of
//the most common, what is the message?
//
//To solve this, we simply look for the minimum counts instead of the maximum.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

//We all know there are 26 letters, but it's still bad practice to put random
//constants in your code.
#define NUM_LETTERS  26
#define MESSAGE_LEN   8


int main(int argc, char **argv)
{
	//Just a variable name change
	FILE *inFile;
	int counts[MESSAGE_LEN][NUM_LETTERS];
	char line[MESSAGE_LEN + 2];
	char message[MESSAGE_LEN+1];
	int c, l, minCount;
	
	memset(counts, 0, MESSAGE_LEN * NUM_LETTERS * sizeof(int));
	memset(line, '\0', MESSAGE_LEN + 2);
	memset(message, '\0', MESSAGE_LEN + 1);

	if (argc != 2)
	{
		fprintf(stderr, "Usage:\n\tDay6 <input filename>\n\n");
		return EXIT_FAILURE;
	}

	inFile = fopen(argv[1], "r");
	if (inFile == NULL)
	{
		fprintf(stderr, "Error opening file: %s\n\n", strerror(errno));
		return EXIT_FAILURE;
	}

	//The counts work the same way
	while (fgets(line, MESSAGE_LEN+1, inFile) != NULL)
	{
		for (c = 0; c < MESSAGE_LEN; c++)
		{
			counts[c][line[c] - 'a']++;
		}
	}
	
	fclose(inFile);
	
	//Instead of looking for the maximum counts, we look for the minimum counts.
	for (c = 0; c < MESSAGE_LEN; c++)
	{
		minCount = INT_MAX;
		for (l = 0; l < NUM_LETTERS; l++)
		{
			if (counts[c][l] < minCount)
			{
				minCount = counts[c][l];
				
				//The last character we write will be the correct one
				message[c] = 'a' + l;
			}
		}
	}
	
	//Print the message
	printf("%s\n", message);
	
	return EXIT_SUCCESS;
}
