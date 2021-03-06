//Day1b.c
//
//Question 2: What is the first location we visit twice? "Location" includes
//any intersection we pass through while following the instructions, not just
//the endpoints.
//
//To solve this puzzle, we can create a 2D array of boolean values. Whenever we
//visit a location, we'll mark it true. When we find a location that's already
//true, that's the answer. Another method could be to make a list of locations
//on the fly and compare each new one against the others. Perhaps there's some
//clever geometric way to solve this without brute force, but I haven't been
//able to think of one.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
	#include <stdbool.h>

//While answering the previous question, we found that the most extreme X and
//Y values were between -256 and 256. Thus, a 512x512 grid can hold every
//location we visit.
	#define GRID_SIZE 512

//I've chosen to represent both the position and direction as vectors in
//Cartesian coordinates. This will make it easier to compute distances.
typedef struct
{
	int x;
	int y;
} sVector;

//Direction constants for convenience
const sVector north = {0, 1};
const sVector south = {0, -1};
const sVector east = {1, 0};
const sVector west = {-1, 0};

//Function prototypes. I like putting the main function first so I can get a
//feel for the whole program before I dive into the supporting functions. The
//downside is that I need to have explicit prototypes before main().
sVector Turn_Right(sVector currentDirection);
sVector Turn_Left(sVector currentDirection);


int main(int argc, char **argv)
{
	sVector direction = north;
	sVector position = {0, 0};
		sVector doubleVisit;
		bool visited[GRID_SIZE][GRID_SIZE];
		bool foundFirst = false;
	FILE *inFile;
	char *input, *token;
	size_t inSize;
	int distance = 0, maxX = 0, maxY = 0, minX = 0, minY = 0;
		int block;

		memset(visited, false, GRID_SIZE * GRID_SIZE * sizeof(bool));
		visited[0+256][0+256] = true;
	
	//We could hard-code the filename, but for this lesson, let's take it as a
	//command line argument. We'll start by checking for the correct number of
	//arguments. The first argument is always the name of the program, so argc
	//should be 2.
	if (argc != 2)
	{
		//We'll use the standard error instead of the standard output for
		//error messages. This lets the user redirect the output without
		//missing any errors. Because we're using a different stream, we
		//need to use fprintf instead of printf.
		fprintf(stderr, "Usage:\n\tDay1 <input filename>\n\n");
		
		//Let's use the stdlib.h constant instead of 1. You never know when
		//your code might need to run on VMS! :-)
		//See: http://stackoverflow.com/a/8868139/5220760
		return EXIT_FAILURE;
	}
	
	//Now we need to open the file. This just takes a call to fopen(). Failure
	//is indicated by a null return value, with an error code stored in errno.
	inFile = fopen(argv[1], "r");
	if (inFile == NULL)
	{
		//We can use strerror() to convert the errno value to a helpful string!
		fprintf(stderr, "Error opening file: %s\n\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
	//Now we need to read the file. We could parse it one character at a time,
	//but I prefer to read all of the data into memory. This makes it easier
	//to tokenize later. But first, we need to allocate memory, and that means
	//we need to know how big the file is. Unfortunately, there's no standard
	//way to do this. We'll have to settle for POSIX compliance instead.
	fseek(inFile, 0, SEEK_END);
	inSize = (size_t)ftell(inFile);
	rewind(inFile);
	
	//With the file size, we can allocate enough memory using malloc(). One
	//more byte is required to hold the null character that will terminate
	//the string.
	input = malloc(inSize + 1);
	if (input == NULL)
	{
		//Always check for NULL when calling malloc()!
		fprintf(stderr, "Error allocating memory: %s\n\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
	//Finally, we can read the data from the file using fgets(). There's no
	//reason to keep the file open any longer, so let's close it now.
	fgets(input, inSize + 1, inFile);
	fclose(inFile);
	
	//Now the real work begins. We need to parse the instructions in the string
	//one at a time. This means we have to break up the string and filter out
	//the separator characters. Our best friend for this task is strtok(), the
	//string tokenizing function. It looks complicated, but it's actually not
	//that bad. We feed in our input string and a list of separator characters,
	//and strtok() returns a pointer to a substring that contains the first
	//instruction with separator characters removed! Calling strtok() again
	//(this time with NULL in place of the string pointer) returns the next
	//token. When the tokens run out, strtok() returns NULL. Note that the
	//original string is modified, so we can't reuse it later.
	//
	//This functionality is similar to the split function in Perl or Python.
	//Those languages helpfully put the tokens into an array, allocating memory
	//in the process, all in a single line of code. That's the benefit of high-
	//level programming! But we're hard-core low-level C programmers, so we
	//don't need any help. :-)
	token = strtok(input, " ,\n");
	while (token != NULL)
	{
		//Having chosen a good representation for position and direction and
		//defined functions for rotation, the actual work is easy!
	
		//The first character of the token string is either 'L' or 'R', which
		//tells us how to rotate. In a real program, we would need an error
		//handler in case the character is something else.
		if (token[0] == 'L')
			direction = Turn_Left(direction);
		else
			direction = Turn_Right(direction);

		//The remaining characters are the distance, and need to be converted
		//to an integer. For this simple case with guaranteed good input, we
		//can use atoi(). In any other situation, we'd have to use strtol().
		distance = atoi(token + 1);

		//The change in position is the new direction multiplied by the
		//distance to walk. To check for double visits, we need to go
		//one block at a time.
			for (block = 0; block < distance; block++)
			{
				position.x += direction.x;
				position.y += direction.y;
			
				//Check whether we've visited this position before
				if (visited[position.x+256][position.y+256] && !foundFirst)
				{
					doubleVisit.x = position.x;
					doubleVisit.y = position.y;
					foundFirst = true;
				} else
				{
					visited[position.x+256][position.y+256] = true;
				}
			}
		
		//Get the extreme X and Y coordinates we see for later use
		if (position.x > maxX)
			maxX = position.x;
		if (position.y > maxY)
			maxY = position.y;
		if (position.x < minX)
			minX = position.x;
		if (position.y < minY)
			minY = position.y;
		
		//Call strtok() again to get a pointer to the next token. If there are
		//no more tokens, strtok() returns NULL.
		token = strtok(NULL, " ,\n");
	}
	
	//Now we have the coordinates of the final location. Adding the absolute
	//X and Y values together gives us the taxicab distance from the starting
	//point.
	printf("Final location: %d, %d\n", position.x, position.y);
	distance = abs(position.x) + abs(position.y);
	printf("Distance from start: %d\n", distance);
	printf("Max X: %d\tMax Y: %d\n", maxX, maxY);
	printf("Min X: %d\tMin Y: %d\n", minX, minY);
		printf("First double visitation: %d, %d\n", doubleVisit.x,
		                                            doubleVisit.y);
		distance = abs(doubleVisit.x) + abs(doubleVisit.y);
		printf("Double visitation distance: %d\n", distance);
	
	//Don't forget to free allocated memory! It happens automatically at the
	//end of the program, but that's no excuse for sloppiness.
	free(input);
	
	return EXIT_SUCCESS;
}


//90-degree rotation is very simple. A general rotation handler would require
//matrix multiplication. You can see what this looks like for the 90-degree
//case here: https://en.wikipedia.org/wiki/Rotation_matrix#Common_rotations
sVector Turn_Right(sVector currentDirection)
{
	sVector newDirection;
	
	newDirection.x = currentDirection.y;
	newDirection.y = -currentDirection.x;
	
	return newDirection;
}

sVector Turn_Left(sVector currentDirection)
{
	sVector newDirection;
	
	newDirection.x = -currentDirection.y;
	newDirection.y = currentDirection.x;
	
	return newDirection;
}
