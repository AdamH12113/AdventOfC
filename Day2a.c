//Day2a.c
//
//The second challenge is to unlock a door by finding a five-digit code on a
//numeric keypad. The keypad is arranged like this:
//
//  +---+---+---+
//  | 1 | 2 | 3 |
//  +---+---+---+
//  | 4 | 5 | 6 |
//  +---+---+---+
//  | 7 | 8 | 9 |
//  +---+---+---+
//
//Our input is a sequence of five lines, one per code digit. The lines contain
//instructions for moving around the keypad starting at 5. After completing all
//the movements, the button we're on is the next digit of the code. Each
//instruction is a single letter -- U (for up), D (for down), L (for left), or
//R (for right). Moving past the edge of the keypad is not allowed, and any
//instruction to do so should be ignored.
//
//Question 1: What is the bathroom code?
//
//To solve this puzzle, we can simulate following the instructions.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

//There are many ways we could simulate the keypad. We could use Cartesian
//coordinates, or (similarly) a 2D array, or even a bunch of switch-case
//statements. Our choice is fraught with peril because we don't know what
//question 2 will be yet! On Day 1 we had to track which locations we had
//visited, so today I'm going to use a structure that makes it easy to store
//more information later -- a multiply-linked list. In this model, each button
//on the keypad is a single node in the list.
//
//Linked lists in C use pointers. A lot of people have trouble with pointers,
//but for an embedded programmer they're very simple -- a pointer is a variable
//that holds the memory address of an object in memory. To understand this, it
//helps to know how CPUs talk to memory. Here's a simplified example:
//
//  +---------+           +---------+
//  |   CPU   |           |   RAM   |
//  |         |  16 bits  |  (64k)  |
//  | Address |-----/---->| Address |
//  |         |  8 bits   |         |
//  |    Data |<----/---->| Data    |
//  |         |           |         |
//  |    R/Wn |---------->| R/Wn    |
//  +---------+   1 bit   +---------+
//
//This pretend RAM has 65536 "words" of memory, and each word is 8 bits. Each
//combination of address bit values selects a single word. Once the word is
//selected, the read/write signal (R/Wn) determines whether the CPU outputs
//data to the RAM (which stores it) or the RAM outputs data to the CPU.
//
//In this example, a pointer is basically an integer variable that holds a
//16-bit address. If the pointer points to something bigger than 8 bits, the
//type of the pointer tells the compiler how many more words it has to read
//to get the whole object. This all comes out in the assembly code that the
//compiler generates; you don't have to worry about it in C. The purpose of
//pointers is to shield the programmer from some of the details of memory
//accesses.
//
//On an embedded system, you can (usually) work directly with memory addresses
//if you want. For example, this code writes the number 42 to the byte at
//memory address 0x1d47:
//
//    *(char *)0x1d47 = 42;
//
//But allowing programs to write to random memory addresses is dangerous,
//especially when multitasking is involved. So PCs running operating systems
//use virtual memory to control access. Memory management is done via dynamic
//allocation (malloc() and friends), or by storing data on the stack.
//
//By the way, CPUs don't only talk to RAM. Those address and data lines can
//also talk to registers, which are single-word storage elements used to
//control hardware. Working with registers is where embedded programming gets
//really fun, but we don't need to talk about that today. Let's make a keypad!
typedef struct sButton
{
	int number;
	
	//Instead of ignoring movements past the edge of the keypad, we'll just
	//have them loop back to the same node. So: one->up == &one. The struct
	//keyword and the odd typedef syntax is needed to keep the compiler from
	//being confused by the presence of "sButton" in the structure definition.
	struct sButton *up, *down, *left, *right;
} sButton;

//The puzzle says that we "picture" a nine-button keypad. I have a feeling that
//the actual keypad in question 2 will have more than nine buttons. That means
//we need to generate our multiply-linked list based on parameters instead of
//hardcoding nine constants.
#define NUM_ROWS    3
#define NUM_COLS    3


int main(int argc, char **argv)
{
	//Note that using typedefs to name structure types is somewhat
	//controversial. See, for example:
	//http://stackoverflow.com/questions/252780/why-should-we-typedef-a-struct-so-often-in-c
	//https://www.kernel.org/doc/Documentation/CodingStyle
	sButton buttons[NUM_ROWS][NUM_COLS];
	sButton *b = &buttons[1][1];
	FILE *inFile;
	int r, c, next;
	int code = 0;
	
	//Generate the list of buttons. C makes multidimensional arrays (mostly)
	//easy to construct and work with. Strangely, some higher-level languages
	//like Python and Perl get weird when you go past two dimensions. I don't
	//know why; Fortran and Matlab handle multidimensional arrays just fine.
	for (r = 0; r < NUM_ROWS; r++)
	{
		for (c = 0; c < NUM_COLS; c++)
		{
			//Number the buttons in order starting at one
			buttons[r][c].number = 1 + r*NUM_COLS + c;
			
			//If the button is on the first row, there's nothing above it, so
			//its up pointer needs to loop back.
			if (r == 0)
				buttons[r][c].up = &buttons[r][c];
			else
				buttons[r][c].up = &buttons[r-1][c];
				
			//Similarly, the first column's left pointer should loop back
			if (c == 0)
				buttons[r][c].left = &buttons[r][c];
			else
				buttons[r][c].left = &buttons[r][c-1];
			
			//The last row's down pointer needs to loop back. Be careful to
			//avoid off-by-one errors when working with the ends of arrays!
			if (r == NUM_ROWS - 1)
				buttons[r][c].down = &buttons[r][c];
			else
				buttons[r][c].down = &buttons[r+1][c];
				
			//The same goes for the last column
			if (c == NUM_COLS - 1)
				buttons[r][c].right = &buttons[r][c];
			else
				buttons[r][c].right = &buttons[r][c+1];
		}
	}
			
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

	//Read the file one character at a time, processing as we go. A newline
	//means that we've reached the next code digit.
	next = fgetc(inFile);
	while (next != EOF)
	{
		if ((char)next == '\n')
		{
			//We have the next digit! Shift the code left one place and add it.
			code = 10*code + b->number;
		} else
		{	
			//Move to the next button. Setting up the data structre in advance
			//means the core program logic is simple!
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
	
		//Read the next character
		next = fgetc(inFile);
	}
	
	//Close the file as soon as we're done with it
	fclose(inFile);
	
	//Print the code
	printf("Door code: %d\n", code);
	
	return EXIT_SUCCESS;
}
