//Day22b.c
//
//Question 2: What is the fewest number of moves required to move the data at
//(x=max, y=0) to (x=0, y=0)?
//
//To solve this puzzle, we need to look at the valid pairs and realize that the
//only valid pairs involve the empty node. Thus, the empty node is the fastest
//way to move any data (including the goal data) around.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


//Cluster node attributes. We only need two, but storing all three will make
//later computations easier.
typedef struct
{
	int size;
	int used;
	int avail;
} sNode;


int main(int argc, char **argv)
{
	//C supports two different kinds of 2D arrays. The first (which we've
	//already used) is a true 2D array that uses a single contiguous block of
	//memory. These are more convenient and efficient, but the downside is that
	//the size of one dimension must be known in advance since the compiler has
	//to know how to compute the memory offsets. A more flexible type uses a 1D
	//array of pointers to 1D arrays. Each sub-array must be initialized
	//separately, but the array can be sized at run-time, and each row can even
	//be a different size! An example of this sort of array is argv, a 1D array
	//of pointers to command line argument strings. Here, we'll be using the
	//latter kind of array so we can size the cluster at run-time.
	sNode **cluster;
	FILE *inFile;
	long viableCount;
	int clusterWidth, clusterHeight, x1, x2, y1, y2, size, used, avail;
		
	//The usual command line argument check and input file opening
	if (argc != 2)
	{
		fprintf(stderr, "Usage:\n\tDay22 <input filename>\n\n");
		return EXIT_FAILURE;
	}

	inFile = fopen(argv[1], "r");
	if (inFile == NULL)
	{
		fprintf(stderr, "Error opening file: %s\n\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
	//Skip the first two lines of the file
	while (fgetc(inFile) != '\n') {;}
	while (fgetc(inFile) != '\n') {;}
	
	//The last line of the input file contains information for the largest
	//cluster. We can use fscanf() to read the file directly until we run out
	//of lines.
	while (fscanf(inFile,
	                 "/dev/grid/node-x%d-y%d     %*dT   %*dT    %*dT   %*d%%\n",
					 &clusterWidth, &clusterHeight) == 2) {;}
	
	//The width and height are one more than the max indices
	clusterWidth++;
	clusterHeight++;
	
	//Allocate memory for the array. First we need an array of pointers (one per
	//X value)...
	cluster = malloc(clusterWidth * sizeof(sNode *));
	if (cluster == NULL)
	{
		fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
	//...then an array of nodes for each Y value.
	for (x1 = 0; x1 < clusterWidth; x1++)
	{
		cluster[x1] = malloc(clusterHeight * sizeof(sNode));
		if (cluster[x1] == NULL)
		{
			fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
			return EXIT_FAILURE;
		}
	}
			
	//Now we can read the file again, this time to initialize the nodes. We have
	//to skip the first two lines again.
	rewind(inFile);
	while (fgetc(inFile) != '\n') {;}
	while (fgetc(inFile) != '\n') {;}
	while (fscanf(inFile,
	                    "/dev/grid/node-x%d-y%d     %dT   %dT    %dT   %*d%%\n",
	                    &x1, &y1, &size, &used, &avail) == 5)
	{
		cluster[x1][y1].size = size;
		cluster[x1][y1].used = used;
		cluster[x1][y1].avail = avail;
	}

	//Close the file as soon as we're done with it
	fclose(inFile);

	//Print a schematic of the maze for manual calculation. :-(
	for (y1 = 0; y1 < clusterHeight; y1++)
	{
		for (x1 = 0; x1 < clusterWidth; x1++)
		{
			if (x1 == 0 && y1 == 0)
				printf("X ");
			else if (y1 == 0 && x1 == clusterWidth - 1)
				printf("G ");
			else if (cluster[x1][y1].used == 0)
				printf("O ");
			else if (cluster[x1][y1].used > 100)
				printf("# ");
			else
				printf(". ");
		}
		printf("\n");
	}
	return EXIT_SUCCESS;

	//Free the cluster
	for (x1 = 0; x1 < clusterWidth; x1++)
	{
		free(cluster[x1]);
	}
	free(cluster);
	
	return EXIT_SUCCESS;
}
