//Day3b.c
//
//Question 2: The groups of three sides lengths are actually in columns, not
//rows. How many possible triangles are there?
//
//This doesn't change much. Instead of processing one line at a time, we'll need
//to process three.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define LINE_LENGTH 16
#define NUM_SIDES   3
#define NUM_COLS    3


int main(int argc, char **argv)
{
	char line[LINE_LENGTH+1];
	char *next;
	FILE *inFile;
	//Using a 2D array and nested loops will save us from having to write a ton
	//of comparisons later.
	long sides[NUM_COLS][NUM_SIDES];
	int s, t;
	int numTriangles = 0;
	
	//The usual command line argument check and input file opening
	if (argc != 2)
	{
		fprintf(stderr, "Usage:\n\tDay2 <input filename>\n\n");
		return EXIT_FAILURE;
	}

	inFile = fopen(argv[1], "r");
	if (inFile == NULL)
	{
		fprintf(stderr, "Error opening file: %s\n\n", strerror(errno));
		return EXIT_FAILURE;
	}

	//Since we're working on three lines at once, we need to change the loop.
	//Instead of doing a single null check after each line, we'll check for the
	//end of file after each group of three lines.
	while (!feof(inFile))
	{
		//We still have to read and parse one line at a time, unless we want to
		//do some funky seeking within the file.
		for (s = 0; s < NUM_SIDES; s++)
		{
			fgets(line, LINE_LENGTH+1, inFile);
			next = line;
			
			//We could have unrolled this loop instead. There would be a small
			//performance improvement and a small code size penalty. Doesn't
			//sound like a big deal until you have to write a program that fits
			//into 16kB of RAM. :-)
			for (t = 0; t < NUM_COLS; t++)
			{
				sides[t][s] = strtol(next, &next, 10);
				
				if (sides[t][s] == 0)
				{
					fprintf(stderr, "Error parsing numbers: %s\n\n",
					                                           strerror(errno));
					fprintf(stderr, "Line: %s\n", line);
					fclose(inFile);
					return EXIT_FAILURE;
				}
			}
		}
		
		//Now we can check all three triangles
		for (t = 0; t < NUM_COLS; t++)
		{
			if (sides[t][0] + sides[t][1] > sides[t][2] &&
			    sides[t][1] + sides[t][2] > sides[t][0] &&
				sides[t][2] + sides[t][0] > sides[t][1])
			{
				numTriangles++;
			}
		}
		
		//Unfortunately, the EOF indicator doesn't get set until we try to read
		//past the end of the file. Using fgetc() and ungetc() lets us do that
		//without losing any data.
		ungetc(fgetc(inFile), inFile);
	}
	
	//Close the file as soon as we're done with it
	fclose(inFile);
	
	//Print the number of triangles
	printf("Number of possible triangles: %d\n", numTriangles);
	
	return EXIT_SUCCESS;
}
