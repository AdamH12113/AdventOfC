//Day18a.c
//
//The eighteenth challenge is to navigate a room full of pressure plate traps.
//The room consists of a grid of tiles with distinct rows. The trapped tiles in
//a row are arranged according to the layout of traps and safe tiles in the
//previous row (closer to the start). Specifically, whether or not a tile is
//trapped depends on the tile in the same column and previous row ("center"), as
//well as the tiles on both sides of that("left" and "right"). A tile in a new
//row is safe if any of the following conditions are met:
//
//* The left and center tiles are traps, but the right tile is not
//* The center and right tiles are traps, but the left tile is not
//* Only the left tile is a trap
//* Only the right tile is a trap
//
//If the new tile is on the edge of the row, the left or right tile that would
//be off the edge of the grid counts as safe.
//
//Our input is a single line of text whose characters represent the state of the
//first row. A period ('.') means safe, and a caret ('^') means trap.
//
//Question 1: Starting with the row in our puzzle input, how many safe tiles are
//there in the first 40 rows?
//
//Question 2: How many safe tiles are there in the first 400000 rows?
//
//To solve this puzzle, we just have to do some logical operations. Since we
//care about three tiles, and each has a boolean state (trap or safe), there are
//eight possible combinations. We can represent these in a sort of truth table:
//
//    Hex L C R | New
//    ----------+----
//    0   S S S | Safe
//    1   S S T | Trap
//    2   S T S | S
//    3   S T T | T
//    4   T S S | T
//    5   T S T | S
//    6   T T S | T
//    7   T T T | S
//
//If we define the trapped state as true, the code for the test would be
//something like:
//
//  if ((L == S && C == S && R == T) ||
//      (L == S && C == T && R == T) ||
//      (L == T && C == S && R == S) ||
//      (L == T && C == T && R == S))
//
//This is not a huge amount of logic, but since the puzzle is so easy let's go a
//little deeper. There's a method for optimizing logic called a Karnaugh Map
//(or K-map for short). A full description is beyond the scope of these
//examples, but the short version is that K-maps help you figure out which
//variables you can ignore in the sub-formulas. The K-map for this problem could
//be drawn like this:
//
//             CR
//         ST TT TS SS
//       \____________
//       |+----+
//     S ||T  T| S  S
//   L   |+----++----+
//     T | S  S |T  T|
//       |      +----+
//
//You draw a rectangle around each group of Ts. Whenever the rectangle covers
//both states of a variable, you can drop it from the formula. For example, the
//top-left box has two Ts, and our code for those was:
//
//    (L == S && C == S && R == T) || (L == S && C == T && R == T)
//
//Notice how the value of C doesn't matter? The K-map gives us an easy visual
//way of recognizing that and combining the two formulas:
//
//    (L == S && R == T)
//
//Similar, the bottom-right box gives:
//
//    (L == T && R == S)
//
//So the new tile is a trap if either L or R is a trap, but not both -- an
//exclusive OR (XOR). That whole huge conditional statement from above just
//became a single CPU instruction!
//
//This gets into an issue with how truth and falsehood are represented in C. C
//defines false as zero and truth as non-zero. This works fine with C's logical
//AND or OR operator (&& and ||), but sadly there isn't a logical XOR. The
//bitwise XOR works, but only if we restrict our booleans to be either 0 or 1!
//Fortunately, the C99 boolean type does this, as do all of C's comparison and
//logical operators. But it is possible to do a bitwise XOR on two "true" ints
//and get "true" as a result! Always be careful when using bitwise operators to
//do logical comparisons.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

int main(int argc, char **argv)
{
	FILE *inFile;
	bool *row1, *row2, *newRow, *oldRow, *temp;
	long numSafe = 0;
	long numRows, rowSize, r, c;
	int nextChar;
	
	//The usual command line argument check and input file opening
	if (argc != 3)
	{
		fprintf(stderr, "Usage:\n\tDay18 <input filename> <row count>\n\n");
		return EXIT_FAILURE;
	}

	inFile = fopen(argv[1], "r");
	if (inFile == NULL)
	{
		fprintf(stderr, "Error opening file: %s\n\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
	//Take the number of rows as an argument to help with test data
	numRows = atoi(argv[2]);
	
	//Get the actual row size from the input (i.e. drop the newline)
	rowSize = 0;
	while (fgetc(inFile) != '\n')
	{
		rowSize++;
	}
	rewind(inFile);
	
	//Allocate memory for two rows. We'll handle the (literal) edge cases by
	//fake safe tiles on either side of the real row data.
	row1 = malloc((rowSize + 2) * sizeof(bool));
	row2 = malloc((rowSize + 2) * sizeof(bool));
	if (row1 == NULL || row2 == NULL)
	{
		fprintf(stderr, "Error allocating memory: %s\n\n", strerror(errno));
		return EXIT_FAILURE;
	}

	//Here's the clever bit -- instead of adding 1 to array indices or starting
	//our loop in a weird place, we're just going to define pointers to the
	//first real tile in each row, which is the second array element. This
	//takes care of the offsets for us, simplifying our code by making negative
	//array indices work. Let's see Python do *that*!
	//
	//I know Python probably has fifteen easier ways of doing it, but shut up
	//and let me enjoy this for a moment. :-)
	//
	//Note: This is different from a notation sometimes used in high-level
	//languages where negative indices are an offset from the end of the array.
	//C neither knows nor cares how big an array is supposed to be -- array
	//accesses are equivalent to pointer arithmetic:
	//
	//    array[i] == *(array + i)
	//    array[-i] == *(array - i)
	oldRow = row1 + 1;
	oldRow[-1] = false;
	oldRow[rowSize-1 + 1] = false;
	newRow = row2 + 1;
	newRow[-1] = false;
	newRow[rowSize-1 + 1] = false;
	
	//Read in the first row, converting to booleans on the fly
	for (c = 0; c < rowSize; c++)
	{
		nextChar = fgetc(inFile);
		if (nextChar == '.')
		{
			oldRow[c] = false;
			numSafe++;
		} else
		{
			oldRow[c] = true;
		}
	}
	
	//Generate 39 more rows for a total of 40
	for (r = 1; r < numRows; r++)
	{
		for (c = 0; c < rowSize; c++)
		{
			//XOR the left and right tiles in the old rose to determine whether
			//the new tile is a trap. Again, the edge cases (c == 0 and c ==
			//rowSize-1 are handled by having actual values past the "edges" of
			//the tile array.
			newRow[c] = oldRow[c-1] ^ oldRow[c+1];
			if (newRow[c] == false)
				numSafe++;
		}
		
		//Swap the row pointers, turning the new row into the next old row.
		temp = oldRow;
		oldRow = newRow;
		newRow = temp;
	}
	
	printf("The number of safe tiles is %ld\n", numSafe);

	return EXIT_SUCCESS;
}
