//Day8b.c
//
//Question 2: The screen displays capital letters, each of which is 5 pixels
//wide and 6 pixels tall. What code is the screen trying to display?
//
//To solve this puzzle, all we have to do is print the array using a fixed-
//width font. The actual modification is only three lines of code near the end,
//but just for fun I'm going to heavily optimize the command processing.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>


#define NUM_ROWS     6
#define NUM_COLUMNS  50
#define LINEBUF_SIZE (24+1)


int main(int argc, char **argv)
{
	//No major changes here...
	FILE *inFile;
	char line[LINEBUF_SIZE];
	bool pixels[NUM_ROWS][NUM_COLUMNS];
	int r, c, p, maxR, maxC, places, numLit;
	bool temp;
	
	//...or here.
	memset(pixels, false, NUM_ROWS * NUM_COLUMNS * sizeof(bool));
	memset(line, '\0', LINEBUF_SIZE);

	//The usual command line argument check and input file opening
	if (argc != 2)
	{
		fprintf(stderr, "Usage:\n\tDay8 <input filename>\n\n");
		return EXIT_FAILURE;
	}

	inFile = fopen(argv[1], "r");
	if (inFile == NULL)
	{
		fprintf(stderr, "Error opening file: %s\n\n", strerror(errno));
		return EXIT_FAILURE;
	}

	//Let's do some of that low-level optimization C is famous for! We can take
	//advantage of the fixed format of the commands to reduce the number of
	//character comparisons. In practice, you would never go this far, but the
	//point here is that knowing the sizes and formats of your data can lead to
	//vast performance improvements. One of the classic examples is a database
	//with fixed field sizes, where the next record is always a constant offset
	//away Compare that to an XML file, where you have to iterate over every
	//single character to get where you want to go. I took this example from
	//Joel on Software. You can find it here as part of a larger article:
	//https://www.joelonsoftware.com/2001/12/11/back-to-basics/
	while (fgets(line, LINEBUF_SIZE, inFile) != NULL)
	{
		//We'll start by ignoring the first character (it's always 'r'). The
		//second character is all we need to determine the command.
		if (line[1] == 'e')
		{
			//The command was "rect". Next, we can get the first number. It
			//always starts at column 5 and is either one or two digits. To
			//determine which, we check the next column for the 'x'. The second
			//number is guaranteed to be one digit by the size of the screen.
			//
			//    012345678
			//    rect CxR
			//    rect CCxR
			if (line[6] == 'x')
			{
				maxC = line[5] - '0';
				maxR = line[7] - '0';
			} else
			{
				maxC = 10*(line[5] - '0') + (line[6] - '0');
				maxR = line[8] - '0';
			}
			
			//That's it. Two comparisons and four or five array accesses. This
			//is probably at least an order of magnitude faster than before. The
			//rest of the code is the same.
			for (r = 0; r < maxR; r++)
			{
				for (c = 0; c < maxC; c++)
				{
					pixels[r][c] = true;
				}
			}
		} else if (line[1] == 'o')
		{
			//The command was "rotate". The next word start in column 7, and the
			//difference between "row" and "column" starts at the beginning.
			//
			//              111
			//    0123456789012
			//    rotate row
			//    rotate column
			if (line[7] == 'r')
			{
				//The word was "row". Rows can only be one digit, and we can
				//find that in column 13. The number of places to rotate starts
				//in column 18 and can be one or two digits. If it's one digit,
				//the next character will be a newline.
				//
				//              1111111111
				//    01234567890123456789
				//    rotate row y=R by N
				//    rotate row y=R by NN
				r = line[13] - '0';
				if (line[19] == '\n')
					places = line[18] - '0';
				else
					places = 10*(line[18] - '0') + (line[19] - '0');
				
				//The rotations are still the same. I've heard that unrolling
				//loop is more trouble than it's worth on superscalar branch-
				//predicting CPUs. In most microcontrollers it's probably still
				//usable, but it works best when you have a fixed loop length,
				//and that's somewhat uncommon.
				for (p = 0; p < places; p++)
				{
					temp = pixels[r][NUM_COLUMNS - 1];
					for (c = NUM_COLUMNS - 1; c > 0; c--)
					{
						pixels[r][c] = pixels[r][c-1];
					}
					pixels[r][0] = temp;
				}
			} else
			{
				//The word was "column". Columns can be one or two digits
				//followed by a space, which we can find starting in column 17.
				//The number of places is guaranteed to be one digit because of
				//the size of the screen, and starts in either column 22 or
				//column 23.
				//
				//              1111111111222
				//    01234567890123456789012
				//    rotate column x=C by N
				//    rotate column x=CC by N
				if (line[17] == ' ')
				{
					c = line[16] - '0';
					places = line[21] - '0';
				} else
				{
					c = 10*(line[16] - '0') + (line[17] - '0');
					places = line[22] - '0';
				}
				
				//The rotations are still the same
				for (p = 0; p < places; p++)
				{
					//Do a single rotation
					temp = pixels[NUM_ROWS - 1][c];
					for (r = NUM_ROWS - 1; r > 0; r--)
					{
						pixels[r][c] = pixels[r-1][c];
					}
					pixels[0][c] = temp;
				}
			}
		} else
		{
			fprintf(stderr, "Error: Unrecognized command %s\n\n", line);
			fclose(inFile);
			return EXIT_FAILURE;
		}
	}		
	
	//Close the file as soon as we're done with it
	fclose(inFile);

	//Count the number of lit pixels and print the "screen" at the same time
	numLit = 0;
	for (r = 0; r < NUM_ROWS; r++)
	{
		for (c = 0; c < NUM_COLUMNS; c++)
		{
			if (pixels[r][c])
			{
				numLit++;
				
				//After some experimentation, I found that 8s were easier to
				//read than Xs or Os.
				printf("8");
			} else
			{
				printf(" ");
			}
		}
		printf("\n");
	}
	printf("\nNumber of lit pixels: %d\n", numLit);
	
	return EXIT_SUCCESS;
}
