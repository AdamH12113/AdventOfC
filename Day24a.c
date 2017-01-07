//Day24a.c
//
//The twenty-fourth challenge is to find the shortest path that touches several
//different points in a series of interconnected ventilation ducts. The ducts
//are a rectangular grid consisting of walls and open spaces, similar to the
//maze on Day 13. However, this maze is both finite and bounded by walls.
//
//Our input is a series of lines, each of which gives the layout of one row of
//the maze. A '#' indicates a wall, a '.' indicates an open space, and a numeral
//represents a must-visit location, with '0' as the starting point. Numbered
//locations are considered to be open spaces for navigation purposes.
//
//Question 1: Starting from location 0, what is the fewest number of steps
//needed to visit every non-zero number on the map at least once?
//
//This penultimate puzzle is arguably the most complex one in the Advent of
//Code, and will require most of our skills to solve. We need to parse the input
//text, allocate memory for the maze, do breadth-first searches to find the
//distances between the target locations, and then find the smallest combination
//of distances. 


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <limits.h>
#include <stdint.h>


//We could define a new queue structure and new functions every time we want a
//queue. (That's what we've been doing, in fact.) C++ includes the Standard
//Template Library (STL) to simplify that sort of thing. But it's also possible
//to make a truly generic container in C. The key is to use pointer typecasting
//and dynamic memory allocation to copy data without knowing its type. The
//downside of this is that we can't pass or return objects by value, which makes
//the user code more complicated. Note that if performance is a concern, you
//could also do something like this with an array instead of a linked list.
typedef struct sQueueNode
{
	struct sQueueNode *next;
	
	//This is a C99 construct called a flexible array member. Effectively, this
	//gives us a pointer to the memory location right after the structure. It's
	//useful for things like variable-length network data packets, or really
	//anything where you have a fixed-size header and variable-size data.
	char nodeData[];
} sQueueNode;
	
typedef struct
{
	size_t numElements, elementSize;
	sQueueNode *first, *last;
} sQueue;

//As on Day 13, we need to keep track of several variables for each location
//("room") in the maze: whether the room is an open space or a wall, whether
//it's been visited already, how far it is from the starting location, and
//whether it's a numbered target location.
typedef struct
{
	bool isOpen, visited;
	int distance;
} sRoom;

//Coordinates for a single room. This is what we'll feed into the queue for the
//breadth-first search.
typedef struct
{
	int x, y;
} sCoord;

//Full information for the maze. This consists of the 2D room array, the size of
//the maze, a list of target coordinates, and the number of target locations.
//The main reason for storing all this in a structure is to make it easier to
//break out the searching and processing code into separate functions.
typedef struct
{
	sRoom **rooms;
	int xSize, ySize;
	sCoord *targets;
	int numTargets;
} sMaze;


sMaze *Create_Maze(FILE *inFile);
void Reset_Maze(sMaze *maze);
void Delete_Maze(sMaze *maze);
void Find_Distances(sMaze *maze, sCoord start, int *distances);
int Find_Shortest_Route(int **distances, int startNode, uint16_t unvisitedMask);
void *Safe_Malloc(size_t size);
void Init_Queue(sQueue *queue, size_t elementSize);
void Enqueue(sQueue *queue, const void *object);
void Dequeue(sQueue *queue, void *object);
void Delete_Queue(sQueue *queue);


//To keep the main function from being atrociously long, I've divided the
//functionality up into several helper functions:
//
//    Create_Maze()     Parse the input file
//    Reset_Maze()      Prepare for a new breadth-first search
//    Find_Distances()  Find the shortest paths to all of the targets from
//                      the given starting coordinates
int main(int argc, char **argv)
{
	FILE *inFile;
	sMaze *maze;
	int **distances;
	int t, shortestRoute;
	
	//The usual command line argument check and input file opening
	if (argc != 2)
	{
		fprintf(stderr, "Usage:\n\tDay24 <input filename>\n\n");
		return EXIT_FAILURE;
	}

	inFile = fopen(argv[1], "r");
	if (inFile == NULL)
	{
		fprintf(stderr, "Error opening file: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
	//Create a maze structure and read the maze data into it. 
	maze = Create_Maze(inFile);
	
	//Close the file as soon as we're done with it.
	fclose(inFile);

	//Okay, now we've got a maze! The next step is to run a series of breadth-
	//first searches to get the shortest path distance between each pair of
	//target locations. There's probably a faster way of doing this, but like
	//I've said before, I'm not an algorithms expert. :-) First, we need to
	//allocate memory for the pairwise list of shortest paths. This is twice as
	//much information as we need, but it simplifies the algorithm greatly, and
	//the cost is low.
	distances = Safe_Malloc(maze->numTargets * sizeof(int *));
	for (t = 0; t < maze->numTargets; t++)
	{
		distances[t] = Safe_Malloc(maze->numTargets * sizeof(int));
	}
	
	//Now we can run the searches, starting at each target location
	for (t = 0; t < maze->numTargets; t++)
	{
		Reset_Maze(maze);
		Find_Distances(maze, maze->targets[t], distances[t]);
	}
	
	//Now that the searches are done, we have a weighted graph of the target
	//locations and the distances between them. We need to find the shortest
	//route that visits every target once. This is a version of the traveling
	//salesman problem. There's a lot of research on approximate solutions, but
	//we need an exact solution, so we'll have to go with a brute-force check of
	//every possible route. This runs in factorial time (O(n!)), so even a dozen
	//nodes would take a long time to process. Fortunately, we only have seven
	//(plus a fixed starting point), so only a few hundred tests are required.
	//Since the number of targets is variable, it's easier to do this with
	//recursion than loops.
	shortestRoute = Find_Shortest_Route(distances, 0,
	                                               (1 << maze->numTargets) - 1);
	
	//Free the maze's memory as soon as we're done with it
	Delete_Maze(maze);
	
	//Print the answer
	printf("Shortest route distance: %d\n", shortestRoute);

	return EXIT_SUCCESS;
}


//Find the shortest route that passes through all of the targets by minimizing
//the sum of the distances. This is done by trying each node distance
int Find_Shortest_Route(int **distances, int startNode, uint16_t unvisitedMask)
{
	int n, minDist, nextDist;
	
	//Mark the current node as visited by clearing its bit in the mask
	unvisitedMask &= ~(1 << startNode);
	
	//If there are no nodes left, the remaining distance is zero
	if (unvisitedMask == 0x0000)
		return 0;
	
	//Start new routes recursively with the current node removed from the list.
	//The shortest total distance will be returned.
	minDist = INT_MAX;
	for (n = 0; n < 16; n++)
	{
		if (unvisitedMask & (1 << n))
		{
			nextDist = distances[startNode][n] +
			                   Find_Shortest_Route(distances, n, unvisitedMask);
							   
			if (nextDist < minDist)
				minDist = nextDist;
		}
	}
	
	return minDist;
}


//Helper function for parsing the input data, allocating memory, and setting up
//the maze structure. For extra fun (and to make life easier for the calling
//function), let's dynamically allocate the maze structure itself.
sMaze *Create_Maze(FILE *inFile)
{
	sMaze *maze;
	int x, y, target, nextChar;
	
	//Allocate memory for the maze structure
	maze = Safe_Malloc(sizeof(sMaze));
	
	//Make sure we're at the start of the file
	rewind(inFile);
	
	//To determine the size of the maze we need to count the characters in one
	//line, then count the number of lines. The first line is all walls, so
	//there's nothing else of interest here.
	maze->xSize = 0;
	while (fgetc(inFile) != '\n')
	{
		maze->xSize++;
	}
	
	//We've already passed the first line, so ySize starts at 1. Since we're
	//reading every character, this is a good place to count the numbered target
	//locations. The targets are numbered starting at zero, so the number of
	//targets is one more than the max value.
	maze->ySize = 1;
	maze->numTargets = 0;
	do
	{
		nextChar = fgetc(inFile);
		if (nextChar == '\n')
		{
			maze->ySize++;
		} else if (nextChar >= '0' && nextChar <= '9')
		{
			target = nextChar - '0';
			if (target + 1 > maze->numTargets)
				maze->numTargets = target + 1;
		}
	} while (nextChar != EOF);
	
	//Allocate memory for the maze-> Since we don't know either dimension at
	//compile time, we have to use the array-of-pointers type. We also now have
	//enough information to allocate memory for the target list and the 2D array
	//of pairwise shortest paths.
	maze->rooms = Safe_Malloc(maze->xSize * sizeof(sRoom *));
	for (x = 0; x < maze->xSize; x++)
	{
		maze->rooms[x] = Safe_Malloc(maze->ySize * sizeof(sRoom));
	}
	maze->targets = Safe_Malloc(maze->numTargets * sizeof(sCoord));
	
	//Read the input file into the maze array. We'll be doing some of the
	//initialization in the search loop, so we don't need to do it all here.
	rewind(inFile);
	for (y = 0; y < maze->ySize; y++)
	{
		for (x = 0; x < maze->xSize; x++)
		{
			nextChar = fgetc(inFile);
			if (nextChar == '#')
			{
				//It's a wall
				maze->rooms[x][y].isOpen = false;
			} else
			{
				//It's an open space. If it's a numbered target, save the
				//coordinates.
				maze->rooms[x][y].isOpen = true;
				if (nextChar >= '0' && nextChar <= '9')
				{
					target = nextChar - '0';
					
					//This is another nifty C99 feature -- a compound literal.
					//Instead of just initializing structures with braced lists
					//of constant values, we can now use constants and even
					//variables to assign values to structures at run-time. This
					//is one of the most high-level features in C -- a single
					//line of code creates a nameless instance of the structure,
					//initializes its members to the provided values, and copies
					//those values into another instance of the structure. The
					//nameless temporary instance is a full lvalue -- we could
					//even create a pointer to it, if we wanted. (We don't.)
					//I'm not going to go crazy with this -- it's probably more
					//trouble than it's worth in more complex situations. For
					//more information about compound literals, see:
					//http://www.drdobbs.com/the-new-c-compound-literals/184401404
					maze->targets[target] = (sCoord){x, y};
				}				
			}
		}
		
		//Drop the newlines
		fgetc(inFile);
	}

	return maze;
}


//Helper function for resetting the room distances and visited flags before
//running a search.
void Reset_Maze(sMaze *maze)
{
	int x, y;
	
	for (x = 0; x < maze->xSize; x++)
	{
		for (y = 0; y < maze->ySize; y++)
		{
			maze->rooms[x][y].visited = false;
			maze->rooms[x][y].distance = UINT_MAX;
		}
	}
}


//Helper function for freeing memory allocated for the maze
void Delete_Maze(sMaze *maze)
{
	int x;
	
	for (x = 0; x < maze->xSize; x++)
	{
		free(maze->rooms[x]);
	}
	free(maze->rooms);
	free(maze->targets);
	free(maze);
}


//Do a breadth-first search on the maze, starting at the given coordinates. Once
//done, record the distance to each of the target locations in the distances
//array.
void Find_Distances(sMaze *maze, sCoord start, int *distances)
{
	//We can always access structure members directly, but sometimes copying
	//them into local variables makes the code easier to read.
	sQueue queue;
	sCoord tempCoord;
	sRoom **rooms;
	int x, y, target;
	
	rooms = maze->rooms;
	
	//Create a queue for coordinates. The queue will be empty at the end of the
	//function, so there's no need to delete it later.
	Init_Queue(&queue, sizeof(sCoord));
	
	//Initialize the starting location and add its coordinates to the queue.
	//We're using a generic queue today, so everything has to be passed and
	//returned by reference.
	maze->rooms[start.x][start.y].distance = 0;
	maze->rooms[start.x][start.y].visited = true;
	Enqueue(&queue, &start);
	
	//Search until the maze is fully explored. This is not the most efficient
	//approach in general, but our particular maze has numbers close to each of
	//the corners, so it's probably not worth the extra comparisons to do an
	//early termination.
	while (queue.numElements > 0)
	{
		//Get the next room
		Dequeue(&queue, &tempCoord);
		x = tempCoord.x;
		y = tempCoord.y;
		
		//Discover adjacent nodes, update their distance, and add them to the
		//queue. Note that since this maze is bounded by walls, we don't have to
		//worry about walking off the edge of the array. See the code from Day
		//13 if you're confused about what's going on here.
		if (rooms[x-1][y].isOpen && !rooms[x-1][y].visited)
		{
			rooms[x-1][y].distance = rooms[x][y].distance + 1;
			rooms[x-1][y].visited = true;
			
			//Note that we could just enqueue the nameless struct created by the
			//compound literal, but nameless variables make me nervous. As an
			//embedded programmer, I'm already nervous about consuming stack
			//space through syntax rather than declarations. Yes, I know I'm
			//fretting about eight bytes of stack space while stuffing dozens of
			//nodes in the queue. :-)
			tempCoord = (sCoord){x-1, y};
			Enqueue(&queue, &tempCoord);
		}
		if (rooms[x+1][y].isOpen && !rooms[x+1][y].visited)
		{
			rooms[x+1][y].distance = rooms[x][y].distance + 1;
			rooms[x+1][y].visited = true;
			tempCoord = (sCoord){x+1, y};
			Enqueue(&queue, &tempCoord);
		}
		if (rooms[x][y-1].isOpen && !rooms[x][y-1].visited)
		{
			rooms[x][y-1].distance = rooms[x][y].distance + 1;
			rooms[x][y-1].visited = true;
			tempCoord = (sCoord){x, y-1};
			Enqueue(&queue, &tempCoord);
		}
		if (rooms[x][y+1].isOpen && !rooms[x][y+1].visited)
		{
			rooms[x][y+1].distance = rooms[x][y].distance + 1;
			rooms[x][y+1].visited = true;
			tempCoord = (sCoord){x, y+1};
			Enqueue(&queue, &tempCoord);
		}
	}
	
	//Now that the search is complete, we can get the shortest path distance to
	//each target by going through the target coordinate list.
	for (target = 0; target < maze->numTargets; target++)
	{
		x = maze->targets[target].x;
		y = maze->targets[target].y;
		distances[target] = rooms[x][y].distance;
	}
}


//Helper function for error-checking malloc()
void *Safe_Malloc(size_t size)
{
	void *retVal;
	
	retVal = malloc(size);
	if (retVal == NULL)
	{
		fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	return retVal;
}


//Initialize the queue. We'll specify the size of the stored objects here for
//simplicity. We could theoretically put the element size in the enqueue
//function, which would let us mix object types in the queue, but that would be
//weird and not very useful.
void Init_Queue(sQueue *queue, size_t elementSize)
{
	queue->numElements = 0;
	queue->elementSize = elementSize;
	queue->first = NULL;
	queue->last = NULL;
}

//Add an object to the queue. Because we don't know the data type, the object
//must be passed by reference as a void pointer.
void Enqueue(sQueue *queue, const void *object)
{
	sQueueNode *new;
	
	//Allocate memory for a new node with enough storage for the object
	new = malloc(sizeof(sQueueNode) + queue->elementSize);
	if (new == NULL)
	{
		fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	//Initialize the list pointer and copy the data into the new node. memcpy()
	//copies one byte at a time, so we don't have to worry about unaligned data.
	new->next = NULL;
	memcpy(new->nodeData, object, queue->elementSize);

	//Add the node to the tail of the list. If the list is empty, we need to
	//update the queue pointers to cover the new list.
	if (queue->numElements > 0)
		queue->last->next = new;
	else
		queue->first = new;
	queue->last = new;
	queue->numElements++;
}

//Remove and return the next node from the queue. Again, not knowing the data
//type means we have to return the data via a pointer.
void Dequeue(sQueue *queue, void *object)
{
	sQueueNode *oldest;
	
	//We can't return anything to indicate an empty queue without knowing the
	//data type, so instead we'll have to crash the program. If we really
	//needed to recover gracefully from an underrun, we could set errno to an
	//appropriate non-zero value (ECANCELED, maybe).
	if (queue->numElements == 0)
	{
		fprintf(stderr, "Error: Queue underrun!\n");
		exit(EXIT_FAILURE);
	}
	
	//Copy the oldest node's data using the provided pointer
	oldest = queue->first;
	memcpy(object, oldest->nodeData, queue->elementSize);
	
	//Delete the oldest node from the linked list. We can only update the queue
	//pointers if there are still other nodes left in the list.
	if (queue->numElements > 1)
		queue->first = oldest->next;
	free(oldest);
	queue->numElements--;
}

//Free all the memory used by the queue. Does nothing if the queue is empty.
void Delete_Queue(sQueue *queue)
{
	sQueueNode *next;
	
	while (queue->numElements > 0)
	{
		next = queue->first->next;
		free(queue->first);
		queue->first = next;
		queue->numElements--;
	}
}
