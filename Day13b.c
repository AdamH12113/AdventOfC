//Day13b.c
//
//Question 2: How many locations (including the starting location) can be
//reached in at most 50 steps?
//
//To solve this puzzle, we just need to explore a subset of the maze, then count
//how many rooms have a distance less than or equal to 50. This means we won't
//actually have to enlarge the maze -- our starting subset is more than 50 rooms
//across, so it's good enough for this problem.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <limits.h>


//No change to the room definition
typedef struct
{
	bool isOpen, visited;
	unsigned long distance;
} sRoom;

//No change to the queue system
typedef struct sCoord
{
	unsigned long x, y;
	struct sCoord *next;
} sCoord;

typedef struct
{
	unsigned long numElements;
	sCoord *first, *last;
} sQueue;

void Init_Queue(sQueue *queue)
{
	queue->numElements = 0;
	queue->first = NULL;
	queue->last = NULL;
}

void Enqueue(sQueue *queue, unsigned long x, unsigned long y)
{
	sCoord *new;
	
	new = malloc(sizeof(sCoord));
	if (new == NULL)
	{
		fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	new->x = x;
	new->y = y;
	new->next = NULL;
	
	if (queue->numElements > 0)
		queue->last->next = new;
	else
		queue->first = new;
	queue->last = new;
	queue->numElements++;
}

sCoord Dequeue(sQueue *queue)
{
	sCoord *oldest;
	sCoord retVal;
	
	if (queue->numElements == 0)
	{
		fprintf(stderr, "Error: Queue underrun!\n");
		exit(EXIT_FAILURE);
	}
	
	oldest = queue->first;
	retVal = *oldest;

	if (queue->numElements > 1)
		queue->first = oldest->next;
	free(oldest);
	queue->numElements--;
	
	return retVal;
}

void Delete_Queue(sQueue *queue)
{
	sCoord *next;
	
	while (queue->numElements > 0)
	{
		next = queue->first->next;
		free(queue->first);
		queue->first = next;
		queue->numElements--;
	}
}


//We'll use our starting subset as the maximum extent of the maze
#define STARTING_SIZE  128

//The starting coordinates are the same
#define STARTX           1
#define STARTY           1


bool Location_Is_Open(unsigned long x, unsigned long y, unsigned long seed);


int main(int argc, char **argv)
{
	//We don't have a target anymore
	sRoom **maze;
	sQueue queue;
	sCoord temp;
	unsigned long input, x, y, xSize, ySize, maxSteps, roomCount;
	
	//Our input will be the maximum number of steps
	if (argc != 3)
	{
		fprintf(stderr, "Usage:\n\tDay13 <input seed> <max steps>\n");
		return EXIT_FAILURE;
	}

	errno = 0;
	input = strtoul(argv[1], NULL, 10);
	maxSteps = strtoul(argv[2], NULL, 10);
	if (errno != 0)
	{
		fprintf(stderr, "Error parsing input: %s\n", strerror(errno));
		fprintf(stderr, "Input should be a number!\n");
		return EXIT_FAILURE;
	}
	
	//The maze initialization is the same
	xSize = STARTING_SIZE;
	ySize = STARTING_SIZE;
	maze = malloc(xSize * sizeof(sRoom *));
	if (maze == NULL)
	{
		fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	for (x = 0; x < xSize; x++)
	{
		maze[x] = malloc(ySize * sizeof(sRoom));
		if (maze[x] == NULL)
		{
			fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
			return EXIT_FAILURE;
		}
	}
	
	for (x = 0; x < xSize; x++)
	{
		for (y = 0; y < ySize; y++)
		{
			maze[x][y].isOpen = Location_Is_Open(x, y, input);
			maze[x][y].visited = false;
			maze[x][y].distance = ULONG_MAX;
		}
	}
	maze[STARTX][STARTY].distance = 0;
	maze[STARTX][STARTY].visited = true;
	
	//We start out the same way
	Init_Queue(&queue);
	Enqueue(&queue, STARTX, STARTY);
	
	//Now we loop until we run out of rooms. This happens when the queue is
	//empty at the end of an iteration.
	while (queue.numElements > 0)
	{
		//Everything else is the same!
		temp = Dequeue(&queue);
		x = temp.x;
		y = temp.y;
		
		if (x > 0 && maze[x-1][y].isOpen && !maze[x-1][y].visited)
		{
			maze[x-1][y].visited = true;
			maze[x-1][y].distance = maze[x][y].distance + 1;
			Enqueue(&queue, x-1, y);
		}
		if (y > 0 && maze[x][y-1].isOpen && !maze[x][y-1].visited)
		{
			maze[x][y-1].visited = true;
			maze[x][y-1].distance = maze[x][y].distance + 1;
			Enqueue(&queue, x, y-1);
		}
		if (x < xSize && maze[x+1][y].isOpen && !maze[x+1][y].visited)
		{
			maze[x+1][y].visited = true;
			maze[x+1][y].distance = maze[x][y].distance + 1;
			Enqueue(&queue, x+1, y);
		}
		if (y < ySize && maze[x][y+1].isOpen && !maze[x][y+1].visited)
		{
			maze[x][y+1].visited = true;
			maze[x][y+1].distance = maze[x][y].distance + 1;
			Enqueue(&queue, x, y+1);
		}
	}
	
	//Now we just count the number of rooms within the specified range
	roomCount = 0;
	for (x = 0; x < xSize; x++)
	{
		for (y = 0; y < ySize; y++)
		{
			if (maze[x][y].distance <= maxSteps)
				roomCount++;
		}
	}
	
	//Print the room count
	printf("Number of rooms within %lu steps: %lu\n", maxSteps, roomCount);

	//Delete the queue and free the maze's memory
	Delete_Queue(&queue);
	for (x = 0; x < STARTING_SIZE; x++)
	{
		free(maze[x]);
	}
	free(maze);
	
	return EXIT_SUCCESS;
}


//Helper function to determine whether a location is an open space (true) or a
//wall (false). No changes here either.
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
