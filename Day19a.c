//Day19a.c
//
//The nineteenth challenge is to determine the result of an odd party game. A
//group of elves sit in order in a circle, each holding one present. Starting
//with elf #1, an elf takes all the presents from the next elf with presents.
//Elves without presents don't get a turn. For example, if there are five elves,
//#1 takes #2's present, #2 is skipped, #3 takes #4's present, #4 is skipped, #5
//takes #1's presents, #1 and #2 are skipped, and #3 takes #5's presents. When
//only one elf remains (#3 in this case) the game is over.
//
//Question 1: With the number of elves given as the input (3012210 for me),
//which elf gets all of the presents?
//
//I feel like there should be a simple formula for this, but I'm not sure what
//it would be. Anyway, let's solve this puzzle by simulating the game. We can
//use a linked list to model the elf circle, and remove nodes when elves have
//their presents taken.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


//Elves have two attributes -- a number representing their position in the
//circle, and a count of how many presents they have. Question 1 doesn't care
//about the present count, but I bet question 2 will, so let's plan ahead.
//Because this is a linked list, we also need a pointer to the next elf.
typedef struct sElf
{
	long position;
	long presents;
	struct sElf *next;
} sElf;


int main(int argc, char **argv)
{
	sElf *circle, *current, *out;
	long numElves, e;
	
	//The usual command line argument processing for a numerical input
	if (argc != 2)
	{
		fprintf(stderr, "Usage:\n\tDay19 <input data>\n\n");
		return EXIT_FAILURE;
	}
	
	//I've been assuming correct input for all of these puzzles. In case you're
	//curious, here's how to error-check a number. strtol() returns 0, LONG_MIN,
	//or LONG_MAX on a failure, all of which could be valid values. (They aren't
	//here, but ignore that for now.) To really check for an error, we need to
	//check errno. We've been using errno to print memory allocation errors, but
	//haven't really talked about it yet. Basically, it's an integer that gets
	//set by some standard library functions when they encounter an error. These
	//functions don't touch errno when there's no error, so we have to set it
	//to zero (no error) ourselves before we check it. If the function we call
	//sets errno to a non-zero value, it means there was an error. We can use
	//strerror() to get a string describing the specific error; there are many
	//standard values. Unfortunately, strtol() doesn't set errno when the
	//conversion fails, so we have to check the returned value as well. Not the
	//greatest example, I guess. :-(
	errno = 0;
	numElves = strtol(argv[1], NULL, 10);
	if (errno != 0)
	{
		fprintf(stderr, "Error parsing argument: %s\n", strerror(errno));
		return EXIT_FAILURE;
	} else if (numElves == 0)
	{
		fprintf(stderr, "Error parsing argument: Must be a positive integer\n");
		return EXIT_FAILURE;
	}

	//Allocate memory for the elves. We could allocate one big block of memory
	//to be an array of elves, but let's have some fun and allocate individual
	//blocks for each elf. That way we can free up memory whenever an elf is
	//removed from the game. (This is grossly inefficient, but it's better for
	//educational purposes.) First, we need to make one elf.
	circle = malloc(sizeof(sElf));
	if (circle == NULL)
	{
		fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	circle->position = 1;
	circle->presents = 1;
	
	//Now that we have a pointer to the first node, we can create the rest of
	//the list.
	current = circle;
	for (e = 1; e < numElves; e++)
	{
		//Allocate memory for the next node
		current->next = malloc(sizeof(sElf));
		if (current->next == NULL)
		{
			fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
			return EXIT_FAILURE;
		}
		
		//Initialize the next node
		current->next->position = e + 1;
		current->next->presents = 1;
		
		//Update the pointer for the next iteration
		current = current->next;
	}
	
	//After the last iteration, we can connect the tail of the list to the head
	//to form the circle.
	current->next = circle;
	
	//Let's play the game! The ending condition is when there's only one elf
	//left in the list, which means its next pointer will point to itself.
	current = circle;
	while (current->next != current)
	{
		//Take the presents from the next elf
		current->presents += current->next->presents;
		
		//Destroy even the memory of the removed elf. Set the current elf's next
		//pointer to skip it, then free the memory. We have to save a temporary
		//pointer to the removed elf for the call to free().
		out = current->next;
		current->next = current->next->next;
		free(out);
		
		//Move to the next elf with presents
		current = current->next;
	}
	
	//Print the answer
	printf("The final elf is #%ld with %ld presents\n", current->position,
	                                                    current->presents);
	
	//He who dies with the most toys wins?
	free(current);
	
	return EXIT_SUCCESS;
}
