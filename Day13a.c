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
//a breadth-first search. In this algorithm, we keep a queue of nodes on the
//frontier of our search. When we evaluate a node, we check all of its exits to
//look for new nodes and check for shorter paths to discovered nodes. Once all
//of the exits of a node have been explored, the node is marked complete.
//
//The tricky part is how to represent and track the nodes. It would be easiest
//to define a large array, which lets us use the coordinates more directly.
//However, the maze is infinitely large, so we might have to travel arbitrarily
//far to find the shortest path. And I have a feeling that part B's target will
//be much farther away. Since a large portion of the nodes will be walls or
//isolated "islands" of open space, we could perhaps save some memory by
//creating an undirected graph. But we'd still probably need an array or list of
//pointers to keep track of the nodes. So let's go with an array for now, and
//we'll deal with part B when we get to it.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <limits.h>


//For brevity, we'll call each location a "room". For each room, we need to keep
//track of whether the room is a wall or an open space, and how far it is from
//the starting node. We could use the distance to indicate whether a node has
//been visited, but for simplicity let's add another boolean flag.
typedef struct
{
	bool isOpen, visited;
	unsigned long distance;
} sRoom;

//The shortest path algorithm requires us to add rooms to a queue as we find
//them. To facilitate this, we'll store queued coordinates in a linked list. I'm
//using longs for everything due to the potentially infinite (read: large)
//nature of the maze.
typedef struct sCoord
{
	unsigned long x, y;
	struct sCoord *next;
} sCoord;

//The queue also has some information of its own. There isn't an easy way to
//make an abstract container in C, so this queue is custom-built to store
//coordinates. Maybe we'll try making an abstract queue another day.
typedef struct
{
	unsigned long numElements;
	sCoord *first, *last;
} sQueue;

//Initialize the coordinate queue
void Init_Queue(sQueue *queue)
{
	queue->numElements = 0;
	queue->first = NULL;
	queue->last = NULL;
}

//Add a coordinate to the queue
void Enqueue(sQueue *queue, unsigned long x, unsigned long y)
{
	sCoord *new;
	
	//Create the new coordinate node for the linked list
	new = malloc(sizeof(sCoord));
	if (new == NULL)
	{
		fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	new->x = x;
	new->y = y;
	new->next = NULL;
	
	//Add the node to the tail of the list. If the list is empty, we need to
	//update the queue pointers to cover the new list.
	if (queue->numElements > 0)
		queue->last->next = new;
	else
		queue->first = new;
	queue->last = new;
	queue->numElements++;
}

//Remove and return the next coordinates from the queue
sCoord Dequeue(sQueue *queue)
{
	sCoord *oldest;
	sCoord retVal;
	
	//In an infinite maze, we should never run out of nodes, so trying to get a
	//node from an empty list is a showstopping error.
	if (queue->numElements == 0)
	{
		fprintf(stderr, "Error: Queue underrun!\n");
		exit(EXIT_FAILURE);
	}
	
	//Copy the oldest node
	oldest = queue->first;
	retVal = *oldest;

	//Delete the oldest node from the linked list. We can only update the queue
	//pointers if there are still other nodes left in the list.
	if (queue->numElements > 1)
		queue->first = oldest->next;
	free(oldest);
	queue->numElements--;
	
	return retVal;
}

//Free all the memory used by the queue
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


//Size of the starting subset of the maze
#define STARTING_SIZE  128

//Starting coordinates
#define STARTX           1
#define STARTY           1


bool Location_Is_Open(unsigned long x, unsigned long y, unsigned long seed);


int main(int argc, char **argv)
{
	//We might need the ability to resize both dimensions of the (known) maze,
	//so we can't use a normal 2D array with one fixed dimension. Instead, we'll
	//use a 1D array of pointers to 1D arrays, which lets us use the 2D array
	//syntax and doesn't require a single block of memory. The downside is that
	//we have to do a lot more initialization and reallocation to handle this.
	sRoom **maze;
	sQueue queue;
	sCoord temp;
	unsigned long input, targetX, targetY, x, y, xSize, ySize;
	
	//No input file this time. We'll take the target coordinates as parameters
	//along with the seed to facilitate use of the test input.
	if (argc != 4)
	{
		fprintf(stderr, "Usage:\n\tDay13 <input seed> <target x> <target y>\n");
		return EXIT_FAILURE;
	}

	//A nonzero errno value tells us that strtol() failed
	errno = 0;
	input = strtoul(argv[1], NULL, 10);
	targetX = strtoul(argv[2], NULL, 10);
	targetY = strtoul(argv[3], NULL, 10);
	if (errno != 0)
	{
		fprintf(stderr, "Error parsing input: %s\n", strerror(errno));
		fprintf(stderr, "Input should be a number!\n");
		return EXIT_FAILURE;
	}
	
	//First, let's initialize a reasonable number of rooms. This means we need
	//to allocate memory for the array of pointers to the columns (x dimension)
	//and the rooms in each column (y dimension).
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
	
	//Now we can initialize the rooms. At this point, we can determine whether
	//every room is a wall or an open space. The only known distance is for the
	//starting location (1,1), which has distance 0.
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
	
	//Initialize the queue and add the starting location as the first element
	Init_Queue(&queue);
	Enqueue(&queue, STARTX, STARTY);
	
	//Finally, we can explore the maze. This specific situation (breadth-first
	//search on an unweighted graph) guarantees that the first time we encounter
	//a room will be along a shortest path, so we don't have to worry about
	//finding every path to the target -- once we encounter the target room, we
	//can terminate immediately.
	while (maze[targetX][targetY].distance == ULONG_MAX)
	{
		//Get the next node from the queue
		temp = Dequeue(&queue);
		x = temp.x;
		y = temp.y;
		
		//Mark the current room as visited
		maze[x][y].visited = true;
		
		//Add unvisited, open, adjacent rooms to the queue. This is the breadth-
		//first part of the search. The maze generation algorithm does not
		//guarantee that the maze is bounded by walls, so we have to make sure
		//not to go off the edges of the array. These compound conditionals are
		//safe because C's short-circuiting behavior guarantees that invalid
		//indices are never evaluated.
		if (x > 0 && maze[x-1][y].isOpen && !maze[x-1][y].visited)
		{
			//Mark the new room as visited to prevent it from being added to the
			//queue multiple times.
			maze[x-1][y].visited = true;

			//The new room is one step away from the current, so its distance is
			//one plus the current distance. Again, this search guarantees that
			//the first time we encounter the room will be on a shortest path.
			maze[x-1][y].distance = maze[x][y].distance + 1;
			Enqueue(&queue, x-1, y);
			
		}
		if (y > 0 && maze[x][y-1].isOpen && !maze[x][y-1].visited)
		{
			//Same logic as above
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
	
	//Print the shortest distance to the target
	printf("Shortest distance to (%lu,%lu): %lu\n", targetX, targetY,
	                                           maze[targetX][targetY].distance);
	
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
