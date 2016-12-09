//Day8a.c
//
//The eighth challenge is to simulate a small LCD screen which is 50 pixels wide
//by 6 pixels tall. The screen can receive three kinds of commands:
//
//    rect nxm                 Turn on all of the pixels in a rectangle n pixels
//                             wide by m pixels tall starting at the top left.
//
//    rotate row y=n by m      Shift all pixels in row n to the right by m
//                             places. The rightmost pixel wraps around to the
//                             left side.
//
//    rotate column y=n by m   Shift all pixels in column n down by m places.
//                             The bottom pixel wraps around to the top.
//
//Our input is a series of lines, each of which has one such command.
//
//Question 1: After executing all of the commands, how many pixels will be lit?
//
//To solve this puzzle, we can use an array of boolean values to represent the
//pixels. The rotate operations are similar to the shift register we made on Day
//7, but we can't reuse that structure because our model is two-dimensional.


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
	//The size of our input lines is well-defined for a change. The longest
	//possible command is:
	//
	//    rotate column y=50 by 6
	//
	//which is 24 characters long including the newline. We can make a fixed-
	//size line buffer as a local variable!
	FILE *inFile;
	char line[LINEBUF_SIZE];
	bool pixels[NUM_ROWS][NUM_COLUMNS];
	char *token;
	int r, c, p, maxR, maxC, places, numLit;
	bool temp;
	
	//Clear the pixel array (and the line buffer, just to be paranoid).
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

	//Read one line at a time
	while (fgets(line, LINEBUF_SIZE, inFile) != NULL)
	{
		//The simple way to parse these commands is to use strtok(), strcmp(),
		//and strtol(). Let's try that for part A and then do something more
		//interesting for part B. First, we determine the basic command using
		//the first word.
		token = strtok(line, " ");
		if (strcmp(token, "rect") == 0)
		{
			//Turn on a rectangle. The next part of the line is two numbers
			//separated by an 'x'. By telling strtok() to treat 'x' as white
			//space, we can separate the numbers into two tokens and call
			//strtol() on each one.
			token = strtok(NULL, " x");
			maxC = (int)strtol(token, NULL, 10);
			token = strtok(NULL, " ");
			maxR = (int)strtol(token, NULL, 10);
			
			//Now that we have the size of the rectangle, we just have to turn
			//on the pixels.
			for (r = 0; r < maxR; r++)
			{
				for (c = 0; c < maxC; c++)
				{
					pixels[r][c] = true;
				}
			}
		} else if (strcmp(token, "rotate") == 0)
		{
			//Rotate something -- but what? The next token tells us.
			token = strtok(NULL, " ");
			if (strcmp(token, "row") == 0)
			{
				//Rotate a row. Next up is "y=n" where n is the row to rotate.
				//Again, we'll use strtok() to get rid of the characters we
				//don't want.
				token = strtok(NULL, " y=");
				r = strtol(token, NULL, 10);
				
				//The same trick works with the number of places to rotate
				token = strtok(NULL, " by");
				places = strtol(token, NULL, 10);
				
				//Now we do the rotations. The outer loop is completely
				//independent from the inner loop.
				for (p = 0; p < places; p++)
				{
					//Do a single rotation. Because we're rotating to the right,
					//we need to start at the last column.
					temp = pixels[r][NUM_COLUMNS - 1];
					for (c = NUM_COLUMNS - 1; c > 0; c--)
					{
						pixels[r][c] = pixels[r][c-1];
					}
					pixels[r][0] = temp;
				}
			} else
			{
				//Rotate a column. This uses the same format as for rows, so the
				//same methods apply.
				token = strtok(NULL, " x=");
				c = strtol(token, NULL, 10);
				
				//Get the number of places to rotate
				token = strtok(NULL, " by");
				places = strtol(token, NULL, 10);

				//Now do the rotations. Again, this is almost identical to the
				//row version. If we wanted to, we could probably merge them
				//with a few extra if statements. This would reduce code size
				//and (maybe) improve maintainability with a small cost to
				//performance.
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
		
	//Print the number of lit pixels
	numLit = 0;
	for (r = 0; r < NUM_ROWS; r++)
	{
		for (c = 0; c < NUM_COLUMNS; c++)
		{
			if (pixels[r][c])
				numLit++;
		}
	}
	printf("Number of lit pixels: %d\n", numLit);
	
	return EXIT_SUCCESS;
}
