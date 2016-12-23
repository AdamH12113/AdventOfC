//Day22a.c
//
//The twenty-second challenge is to analyze the data usage of a storage cluster.
//The nodes in the cluster are arranged in a rectangular grid, and data can only
//be transferred between adjacent nodes. We only have access to the data at
//(0,0), but we can instruct other nodes to move all of their data to an
//adjacent node.
//
//Our input is a list of nodes by X and Y coordinate in the grid, along with
//information about their total storage and how much is used and available. The
//information for each node is on one line, and uses a format similar to Unix's
//df utility:
//
//    /dev/grid/node-xN-yM     ssT   uuT    aaT   pp%
//
//where N and M are the X and Y coordinates, ss is the total storage space,
//uu is the used space, aa is the available space, and pp is the use percentage.
//
//Question 1: A viable pair of nodes is defined as two separate nodes (A,B) such
//that node A is not empty (used > 0), and the data on node A would fit on node
//B (A.used < B.available). The nodes do not have to be adjacent to each other.
//How many viable pairs of nodes are there?
//
//To solve this puzzle, we have to do a pairwise comparison of each node.


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

	//Now we can count the viable pairs. We just have to compare each pair of
	//nodes. Again, the rules are:
	//
	//1. Node A is not empty.
	//2. Node A and B are not the same node.
	//3. The data on node A (used) would fit on node B (avail).
	viableCount = 0;
	for (x1 = 0; x1 < clusterWidth; x1++)
	{
		for (y1 = 0; y1 < clusterHeight; y1++)
		{
			for (x2 = 0; x2 < clusterWidth; x2++)
			{
				for (y2 = 0; y2 < clusterHeight; y2++)
				{
					//Don't compare a node to itself
					if (x1 == x2 && y1 == y2)
						continue;
					
					//Don't count empty nodes
					if (cluster[x1][y1].used == 0)
						continue;
					
					//Increment the count if this pair is viable
					if (cluster[x1][y1].used < cluster[x2][y2].avail)
						viableCount++;
				}
			}
		}
	}
	
	//The other downside of using this kind of array is that we have to free
	//each sub-array individually.
	for (x1 = 0; x1 < clusterWidth; x1++)
	{
		free(cluster[x1]);
	}
	free(cluster);
	
	//Print the count of viable nodes
	printf("Number of viables node: %ld\n", viableCount);
	
	return EXIT_SUCCESS;
}
