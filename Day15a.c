//Day15a.c
//
//The fifteenth challenge is to figure out when a group of rotating discs with
//slots in them align in such a way that a small capsule can fall through the
//slots to the bottom. Each disc has multiple positions, which are numbered
//starting at zero. One position (position zero) on each disc has a slot. The
//discs move to the next position once per second. The capsule takes one second
//to fall to the next disc, and starts one second above the first disc.
//
//Our input is a series of lines, each of which describes one disc -- its
//order in the stack, its number of position, and what position it's in to start
//with. The format is completely fixed:
//
//    Disc #N has P positions; at time=0; it is as position S
//
//where N is the distance from the top, P is the number of positions and S is
//the starting position.
//
//Question 1: What is the earliest time we can release a capsule and have it
//fall through all the discs?
//
//This seems like something we should be able to derive a huge equation for, but
//this is a programming puzzle, so let's simulate the discs.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>


//Each disc has two attributes -- the number of positions, and the current
//position. We'll represent the order in the stack with an array.
typedef struct
{
	int numPos;
	int curPos;
} sDisc;


//We could read the file to extract the number of discs, but let's make this a
//short day and hard-code the number.
#define NUM_DISCS     6

//Format specifier for one line of input. Because we're reading from the file
//directly, we need to include the newline. This might be an artifact of the
//two-character Windows newline format.
#define FORMAT  "Disc #%d has %d positions; at time=0, it is at position %d.\n"


int main(int argc, char **argv)
{
	FILE *inFile;
	sDisc discs[NUM_DISCS];
	int disc, positions, start, time, d;
	bool aligned;
	
	//The usual command line argument check and input file opening
	if (argc != 2)
	{
		fprintf(stderr, "Usage:\n\tDay15 <input filename>\n\n");
		return EXIT_FAILURE;
	}

	inFile = fopen(argv[1], "r");
	if (inFile == NULL)
	{
		fprintf(stderr, "Error opening file: %s\n\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
	//Read the file one line at a time. Because the format is so rigid, we can
	//use fscanf() to read and parse at the same time.
	while (fscanf(inFile, FORMAT, &disc, &positions, &start) == 3)
	{
		//Initialize the next disc
		discs[disc-1].numPos = positions;
		discs[disc-1].curPos = start;
	}

	//For the capsule to fall through, the slots need to be staggered -- disc
	//2's slot should come one second after disc 1's, disc 3's slot should come
	//one second after that, and so on. Disc 1's slot should be one second away
	//to account for the time it takes the capsule to get there after release.
	aligned = false;
	time = 0;
	while (!aligned)
	{
		//Step forward one second, which advances the discs one position. As
		//usual with rotation, we'll use modulus division to handle the return
		//to position 0.
		for (d = 0; d < NUM_DISCS; d++)
		{
			discs[d].curPos = (discs[d].curPos + 1) % discs[d].numPos;
		}
		time++;

		//Check for alignment. The rotation toward positive positions and the
		//fact that some discs have only a few positions makes the math a bit
		//tricky. We need to make sure the comparison value doesn't underflow,
		//so we apply the modulus before the subtraction instead of after.
		aligned = true;
		for (d = 0; d < NUM_DISCS; d++)
		{
			if (discs[d].curPos != discs[d].numPos - ((d+1) % discs[d].numPos))
				aligned = false;
		}
	}
	
	//Print the time
	printf("The first possible release time is %d\n", time);		
	
	return EXIT_SUCCESS;
}
