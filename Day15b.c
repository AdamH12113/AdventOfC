//Day15b.c
//
//Question 2: The discs have been reset to the starting positions and a new disc
//(11 positions, starts at 0) has been added to the bottom. With this new disc,
//what is the first valid time we can release a capsule?
//
//This is a straightforward extension of the previous problem.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>


//No change to the disc model
typedef struct
{
	int numPos;
	int curPos;
} sDisc;


//We now have one more disc than what's in the file
#define NUM_DISCS     (6 + 1)

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
	
	//Since we used fscanf() as the loop condition, no changes are needed here
	while (fscanf(inFile, FORMAT, &disc, &positions, &start) == 3)
	{
		//Initialize the next disc
		discs[disc-1].numPos = positions;
		discs[disc-1].curPos = start;
	}

	//Add the new disc
	discs[NUM_DISCS-1].numPos = 11;
	discs[NUM_DISCS-1].curPos = 0;
	
	//The slot conditions are the same
	aligned = false;
	time = 0;
	while (!aligned)
	{
		//Disc rotation is the same
		for (d = 0; d < NUM_DISCS; d++)
		{
			discs[d].curPos = (discs[d].curPos + 1) % discs[d].numPos;
		}
		time++;

		//The alignment check is also the same
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
