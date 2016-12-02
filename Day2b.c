//Day2b.c
//
//Question 2: The keypad is this weird diamond-shaped thing. What is the
//bathroom code?
//
//          +---+
//          | 1 |
//      +---+---+---+
//      | 2 | 3 | 4 |
//  +---+---+---+---+---+
//  | 5 | 6 | 7 | 8 | 9 |
//  +---+---+---+---+---+
//      | A | B | C |
//      +---+---+---+
//          | D |
//          +---+
//
//Well, crud. We planned for a bigger keypad, not a totally different shape.
//But the basic idea of using a multiply-linked list is still sound. There are
//a couple ways we can modify the program. One would be to keep the 2D array of
//sButtons and ignore the ones in the corners. It's not very efficient, but this
//isn't a very big problem. Alternately, we could switch to a 1D array with some
//more complicated logic. Or we could do something weird like make a 2D array of
//pointers and only allocate memory for the buttons that exist (more efficent!).
//Since we're already playing with pointers, let's go with the last one.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


//The structure is the same as last time
typedef struct sButton
{
	int number;
	
	struct sButton *up, *down, *left, *right;
} sButton;

//Sure enough, these had to change. We'll need extra logic too.
#define NUM_ROWS    5
#define NUM_COLS    5


void Free_Buttons(sButton *buttons[NUM_ROWS][NUM_COLS]);

	
int main(int argc, char **argv)
{
	//Instead of an array of structs, we have an array of pointers to structs.
	//Pointer syntax can be confusing, so sometimes it helps to parenthesize.
	sButton *buttons[NUM_ROWS][NUM_COLS];
	sButton *b;
	FILE *inFile;
	int r, c, next;
	int code = 0;
	int nextNumber = 1;
	
	//Initialize the pointer array, allocating memory for the real buttons.
	//Doing this in a separate loop simplifies later code.
	for (r = 0; r < NUM_ROWS; r++)
	{
		for (c = 0; c < NUM_COLS; c++)
		{
			//The missing buttons are ones where the taxicab distance from the
			//center is more than two.
			if (abs(NUM_ROWS/2 - r) + abs(NUM_COLS/2 - c) > 2)
			{
				buttons[r][c] = NULL;
			} else
			{
				buttons[r][c] = malloc(sizeof(sButton));
				if (buttons[r][c] == NULL)
				{
					fprintf(stderr, "Error allocating memory: %s\n\n",
					                                          strerror(errno));
					Free_Buttons(buttons);
					return EXIT_FAILURE;
				}
			}
		}
	}
	
	//Fill out the structures for the real buttons
	for (r = 0; r < NUM_ROWS; r++)
	{
		for (c = 0; c < NUM_COLS; c++)
		{
			//Skip the nonexistent buttons
			if (buttons[r][c] == NULL)
				continue;
		
			//I don't know of an easy algorithm to determine the button number,
			//so let's use an index variable instead. "But wait!" you say, "the
			//last four buttons are letters now!" Sure, but they also work as
			//hexadecimal values. And embedded programmers love hex!
			buttons[r][c]->number = nextNumber++;
			
			//Our loopback checks now look for both the edge of the array and
			//an adjacent NULL pointer. Either one counts as a keypad boundary.
			//C's short-circuiting behavior should guarantee that we'll never
			//have an array underrun. In a statement like this:
			//
			//    if (A || B)
			//
			//If A is true, than B should never be evaluated. In security-
			//critical code, I'd be worried about an overzealous optimizer
			//evaluating it anyway, but here it should be okay.
			if (r == 0 || buttons[r-1][c] == NULL)
				buttons[r][c]->up = buttons[r][c];
			else
				buttons[r][c]->up = buttons[r-1][c];
				
			//Left
			if (c == 0 || buttons[r][c-1] == NULL)
				buttons[r][c]->left = buttons[r][c];
			else
				buttons[r][c]->left = buttons[r][c-1];
			
			//Down
			if (r == NUM_ROWS - 1 || buttons[r+1][c] == NULL)
				buttons[r][c]->down = buttons[r][c];
			else
				buttons[r][c]->down = buttons[r+1][c];

			//Right
			if (c == NUM_COLS - 1 || buttons[r][c+1] == NULL)
				buttons[r][c]->right = buttons[r][c];
			else
				buttons[r][c]->right = buttons[r][c+1];
		}
	}
	
	//We can initialize b once all the pointer values are fixed
	b = buttons[2][0];
	
	//The rest is almost identical except for the final output. First, we check
	//the command line arguments and open the file...
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

	//...then we read the file one character at a time...
	next = fgetc(inFile);
	while (next != EOF)
	{
		if ((char)next == '\n')
		{
			//Here's the change. Because we're working with hexadecimal digits,
			//we need to shift left one hex place instead of one decimal place.
			//Instead of multiplying by 10, we multiply by 16.
			code = 16*code + b->number;
		} else
		{
			switch ((char)next)
			{
				case 'U': b = b->up;    break;
				case 'D': b = b->down;  break;
				case 'L': b = b->left;  break;
				case 'R': b = b->right; break;
				default:
					fprintf(stderr, "Error: Unexpected character %c\n\n", next);
					fclose(inFile);
					return EXIT_FAILURE;
			}
		}		

		next = fgetc(inFile);
	}
	
	//...finally, we close the file, free the memory, and print the code.
	fclose(inFile);
	Free_Buttons(buttons);
	
	//Note that the code is printed as hex instead of decimal (%x instead of %d)
	printf("Door code: %x\n", code);

	return EXIT_SUCCESS;
}


//free() does nothing if we pass it a NULL pointer, so we can call it on the
//whole array to free any allocated memory.
void Free_Buttons(sButton *buttons[NUM_ROWS][NUM_COLS])
{
	int r, c;
	
	for (r = 0; r < NUM_ROWS; r++)
	{
		for (c = 0; c < NUM_ROWS; c++)
		{
			free(buttons[r][c]);
		}
	}
}
