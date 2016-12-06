//Day4b.c
//
//The letters before the sector ID represent the encrypted name of a room. The
//encryption is a shift cipher. The shift is equal to the room's sector ID.
//Hyphens become spaces.
//
//Question 2: What is the sector ID of the room where North Pole objects are
//stored?
//
//To solve this puzzle, we have to decrypt the names of the valid rooms. This
//means we now have to save the line of text. If we keep our assumption that a
//line could be arbitrarily long, we have to read through each line to figure
//out its length, then rewind to the start of the line, allocate memory for a
//buffer, and then re-read the line into the buffer. Alternately, we could
//allocate memory for the entire file at once. We already did that one Day 1,
//so let's use the latter method to get some practice with memory management.
//This will also give you an idea of how much work high-level languages can
//do behind the scenes, whether you want them to or not! See, for instance:
//http://stupidpythonideas.blogspot.com/2013/06/readlines-considered-silly.html


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#define NUM_LETTERS  26
#define CHKSUM_LEN   5


void Decode_Room_Name(char *name, int id);
void Calc_Chksum(int *counts, char *chksum, int numCounts, int chksumSize);

int main(int argc, char **argv)
{
	FILE *inFile;
	int counts[NUM_LETTERS];
	char calcChksum[CHKSUM_LEN];
	char *readChksum;
	char *roomName;
	int nextChar, lineLength, bufSize;
	long idSum = 0;
	long id;

	//The usual command line argument check and input file opening
	if (argc != 2)
	{
		fprintf(stderr, "Usage:\n\tDay4 <input filename>\n\n");
		return EXIT_FAILURE;
	}

	inFile = fopen(argv[1], "r");
	if (inFile == NULL)
	{
		fprintf(stderr, "Error opening file: %s\n\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
	//We still have to do the same tasks as before, but now we need to store
	//the line text along the way. To do this, we're going to keep track of
	//the current line's length and how much memory we've allocated. Whenever
	//we run out of memory, we'll call realloc() to get more. The usual
	//recommendation for realloc() is to round up to the next power of two when
	//you need more memory, so let's do that. To show how it works, we'll start
	//out with 2 bytes and print a message to stderr whenever we need more.
	bufSize = 2;
	roomName = malloc(bufSize * sizeof(char));
	if (roomName == NULL)
	{
		fprintf(stderr, "Error allocating memory: %s\n\n", strerror(errno));
		fclose(inFile);
		return EXIT_FAILURE;
	}
	
	//Now we can start the actual loop
	nextChar = '\n';
	do
	{
		if (nextChar == '\n')
		{
			//Reset everything on a new line. This time we also clear out our
			//buffer and reset the line length.
			memset(counts, 0, NUM_LETTERS * sizeof(int));
			memset(calcChksum, '\0', CHKSUM_LEN * sizeof(char));
			memset(roomName, '\0', bufSize * sizeof(char));
			id = 0;
			lineLength = 0;
		} else if (nextChar >= 'a' && nextChar <= 'z')
		{
			//We don't need to save the checksum separately anymore
			counts[nextChar - 'a']++;
			
			//Save the letters here. We'll handle the hyphens next.
			roomName[lineLength-1] = (char)nextChar;
		} else if (nextChar == '-')
		{
			roomName[lineLength-1] = ' ';
		} else if (nextChar >= '0' && nextChar <= '9')
		{
			//There's no need to change the ID handling. Using strtol() would
			//only complicate things.
			id = 10*id + (nextChar - '0');
		} else if (nextChar == '[')
		{
			//We're going to store the provided checksum in the same buffer as
			//the room name. We'll split the buffer by writing a null character
			//in place of a '['. We still need a separate pointer to pass to
			//Calc_Chksum(). The buffer index will be the character after this
			//one.
			//
			//Adding an index value to an array pointer produces a pointer to
			//that array index. I've been using lineLength-1 for the current
			//character, so I add one to that. Even though the -1 and the +1
			//cancel out, I still like to write them out to be more clear about
			//what I'm doing. The compiler optimizes it out for us.
			readChksum = roomName + (lineLength-1) + 1;
			roomName[lineLength - 1] = '\0';
		} else if (nextChar == ']')
		{
			//No change to our checksum handling...
			Calc_Chksum(counts, calcChksum, NUM_LETTERS, CHKSUM_LEN);
			
			//...or the comparison.
			if (strncmp(calcChksum, readChksum, CHKSUM_LEN) == 0)
			{
				idSum += id;

				//We only want to decode and print valid room names that contain
				//the word "north". To check for "north", we can use the
				//hilariously-named strstr(). strstr() returns a pointer to the
				//substring if it finds it or a null pointer if not.
				Decode_Room_Name(roomName, id);
				if (strstr(roomName, "north") != NULL)
					printf("%s ID:%ld\n", roomName, id);
			}
		}
		
		//Here's where we handle the memory management. Before we read a new
		//character, we increment the line length and check whether we need more
		//buffer space. If we do, we call realloc() with the rounded up buffer
		//size. There's no change to the existing data in the buffer.
		lineLength++;
		if (lineLength > bufSize)
		{
			bufSize *= 2;
			if (realloc(roomName, bufSize * sizeof(char)) == NULL)
			{
				fprintf(stderr, "Error reallocating: %s\n\n", strerror(errno));
				fclose(inFile);
				free(roomName);
				return EXIT_FAILURE;
			}
			
			fprintf(stderr, "New buffer size: %d\n", bufSize);
		}
	} while ((nextChar = fgetc(inFile)) != EOF);	
	
	//Close the file and free the memory as soon as we're done with it
	free(roomName);
	fclose(inFile);
	
	//Print the sum of the sector IDs
	printf("Sum of sector IDs: %ld\n", idSum);
	
	return EXIT_SUCCESS;
}


//Decode the room name using the sector ID as the shift
void Decode_Room_Name(char *name, int id)
{
	do
	{
		if (*name != ' ')
		{
			//Any time you need to rotate through a list of numbers, you're
			//going to use modulus division. This is no exception. The tricky
			//part is making sure we use the right numbers in the first place!
			//The formula we want is:
			//
			//    newLetter = (oldLetter + id) % 26
			//
			//but we can't use the plain character -- 'a' is equal to 97!
			//Instead, we need to use the offset from 'a':
			//
			//    newOffset = (oldOffset + id) % 26
			//
			//then add it to 'a' to get a letter.
			*name = (((*name - 'a') + id) % 26) + 'a';
		}
		
		//Move to the next character. Stop when we hit the null termination.
		name++;
	} while (*name != '\0');
}


//The checksum function is unchanged
void Calc_Chksum(int *counts, char *chksum, int numCounts, int chksumSize)
{
	int max = 0, s = 0;
	int c;
	
	for (c = 0; c < numCounts; c++)
	{
		if (counts[c] > max)
			max = counts[c];
	}
	
	while (s < chksumSize)
	{
		for (c = 0; c < numCounts && s < chksumSize; c++)
		{
			if (counts[c] == max)
			{
				*chksum++ = 'a' + c;
				s++;
			}
		}
		
		max--;
	}
}


