//Day13a.c
//
//The thirteenth challenge is to traverse a maze whose layout is determined by a
//mathematical formula. The maze is described with (x,y) coordinates. We start
//at (1,1). A location in the maze can be either an open space or a wall. To
//determine which is which, we use the following method:
//
//1. Compute x^2 + 3x + 2xy + y + y^2.
//2. Add our input, which is a single number (1352 for me).
//3. Count of number of 1 bits in the binary representation of the result.
//4. If the number of bits is even, the location is an open space, otherwise
//   it's a wall.
//
//Question 1: What is the fewest number of steps required to reach (31,39)?
//
//To solve this puzzle, we need to find the right algorithm. And the right
//algorithm for this sort of thing is Dijkstra's shortest path algorithm. Since
//the distance between adjacent rooms is always 1, we can use a simplfication --
//a breadth-first search.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


int main(int argc, char **argv)
{
	long input;
	
	//No input file this time, just a number
	if (argc != 2)
	{
		fprintf(stderr, "Usage:\n\tDay13 <input data>\n\n");
		return EXIT_FAILURE;
	}

	//A nonzero errno value tells us that strtol() failed
	errno = 0;
	input = strtol(argv[1], NULL, 10);
	if (errno != 0)
	{
		fprintf(stderr, "Error parsing input: %s\n", strerror(errno));
		fprintf(stderr, "Input should be a number!\n\n");
		return EXIT_FAILURE;
	}
	

		
	return EXIT_SUCCESS;
}


//Helper function to determine whether a location is an open space (true) or a
//wall (false).
bool Location_Is_Open(unsigned long x, unsigned long y, unsigned long seed)
{
	long temp;
	
	//First, compute the value of x^2 + 3x + 2xy + y + y^2
	temp = x*x + 3*x + 2*x*y + y + y*y;
	
	//Then add the puzzle input
	temp += seed;
	
	//Now find the number of 1 bits in the binary representation. This operation
	//is called a population count, or popcount for short. (If you want to be
	//really fancy you can refer to the number of 1 bits as the Hamming Weight.)
	//Some CPUs have a popcount instruction, but not all. Instead of using
	//x86 assembly or a GCC built-in function, let's use a more compatible
	//algorithm. This comes from the excellent Hacker's Delight by Henry Warren.
	//It adds pairs of bits and stores the results in those bits, then repeats
	//the process on group of two bits, four bits, eight bits, etc.
	temp = (temp & 0x55555555) + ((temp >> 1) & 0x55555555);
	temp = (temp & 0x33333333) + ((temp >> 2) & 0x33333333);
	temp = (temp & 0x0f0f0f0f) + ((temp >> 4) & 0x0f0f0f0f);
	temp = (temp & 0x00ff00ff) + ((temp >> 8) & 0x00ff00ff);
	temp = (temp & 0x0000ffff) + ((temp >> 16) & 0x0000ffff);

	//If the number of bits is even, this is an open space; otherwise, it's a
	//wall.
	return (temp % 2 == 0);
}
