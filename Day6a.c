//Day6a.c
//
//The sixth challenge is to decode a corrupted message that is sent using a
//repetition code. This is the same message sent over and over with different
//corruption every time.
//
//Our input is a series of lines, each of which contains one attempt to send the
//message -- a string of 8 letters. The actual message is made up of the most
//common letters that appear in each position of the message across the entire
//input.
//
//Question 1: What is the message?
//
//To solve this, we need to keep track of character counts, similar to what we
//did on Day 4. This day is easy!

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

//We all know there are 26 letters, but it's still bad practice to put random
//constants in your code.
#define NUM_LETTERS  26
#define MESSAGE_LEN   8


int main(int argc, char **argv)
{
	//The message length buffer needs to hold both the newline and the null
	//terminator in addition to the message. If we wanted to, we could 
	FILE *inFile;
	int counts[MESSAGE_LEN][NUM_LETTERS];
	char line[MESSAGE_LEN + 2];
	char message[MESSAGE_LEN+1];
	int c, l, maxCount;
	
	//Clear all the arrays
	memset(counts, 0, MESSAGE_LEN * NUM_LETTERS * sizeof(int));
	memset(line, '\0', MESSAGE_LEN + 2);
	memset(message, '\0', MESSAGE_LEN + 1);

	//The usual command line argument check and input file opening
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

	//Read the lines one at a time and count the letters
	while (fgets(line, MESSAGE_LEN+1, inFile) != NULL)
	{
		for (c = 0; c < MESSAGE_LEN; c++)
		{
			//The counts are stored in a 2D array. The first dimension is the
			//position within the message. The second dimension is the letter.
			//To get the letter offset, we can subtract the ASCII value of 'a'
			//from the line character.
			counts[c][line[c] - 'a']++;
		}
	}
	
	//Close the file as soon as we're done with it
	fclose(inFile);
	
	//Find the most common letters by iterating over the count array, looking
	//for the maximum counts.
	for (c = 0; c < MESSAGE_LEN; c++)
	{
		maxCount = 0;
		for (l = 0; l < NUM_LETTERS; l++)
		{
			if (counts[c][l] > maxCount)
			{
				maxCount = counts[c][l];
				
				//The last character we write will be the correct one
				message[c] = 'a' + l;
			}
		}
	}
	
	//Print the message
	printf("%s\n", message);
	
	return EXIT_SUCCESS;
}
