//Day19b.c
//
//Question 2: The elves now steal presents from the elf directly across from
//them in the circle (meaning halfway around the circle, rounded to the closer
//elf). Which elf gets all the presents now?
//
//Now instead of removing the next node in the list, we need to skip halfway
//around the circle. This means we need to keep track of how many elves are
//left, which is pretty easy to do. Getting halfway around the circle (rounded
//down) is also easy, because rounding down is the behavior of integer division.
//There is, however, one huge problem to overcome, which I'll discuss below.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


//The elf structure doesn't change
typedef struct sElf
{
	long position;
	long presents;
	struct sElf * restrict next;
} sElf;


int main(int argc, char **argv)
{
	//We need an extra pointer because we now have to work with three different
	//elves -- the one who steals the present, the one who gets deleted, and the
	//one right before that whose next pointer needs to be updated.
	sElf *circle, *current, *out, *pre;
	long numElves, e, separation;
	
	//The usual command line argument processing for a numerical input
	if (argc != 2)
	{
		fprintf(stderr, "Usage:\n\tDay19 <input data>\n\n");
		return EXIT_FAILURE;
	}
	
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

	//We still create elves the same way. Start with one...
	circle = malloc(sizeof(sElf));
	if (circle == NULL)
	{
		fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	circle->position = 1;
	circle->presents = 1;
	
	//...then construct the rest of the list from there.
	current = circle;
	for (e = 1; e < numElves; e++)
	{
		current->next = malloc(sizeof(sElf));
		if (current->next == NULL)
		{
			fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
			return EXIT_FAILURE;
		}
		
		current->next->position = e + 1;
		current->next->presents = 1;
		
		current = current->next;
	}
	current->next = circle;

	//Now we have to find both the next elf (for present theft), and the one
	//before it (for list modification). Finding the latter elf gets us
	//both. In our previous program, we used the next pointers to find the
	//next elf. The simplest modification would be to walk the linked list,
	//going from next pointer to next pointer until we find the elf at the
	//position we want. This is technically correct in that it will find the
	//solution. The problem is that it takes forever! With an input on the
	//order of 3 million, we have to take 1.5 million steps just to find the
	//target elf! On my computer (a Core i7 running at 3.5 GHz), the program
	//starts out processing a few hundred elves per second.
	//
	//The easiest solution is to keep a pointer to the opposite side of the
	//circle and update it whenever presents are taken. We also need to keep
	//track of the number of elves between the pointers so we can adjust for the
	//changing size of the circle.
	current = circle;
	pre = circle;
	for (e = 0; e < numElves/2 - 1; e++)
	{
		pre = pre->next;
	}
	separation = numElves/2 - 1;
	
	//With the new pointer set up, we can play the game. Since we're tracking
	//the number of elves, the condition is a bit simpler this time.
	while (numElves > 1)
	{
		//Take the presents from the target elf
		out = pre->next;
		current->presents += out->presents;
		
		//Removing the target from the list works the same way as before, except
		//the previous elf in the list is no longer the current elf.
		pre->next = out->next;
		free(out);
		
		//Move to the next elf with presents. This decreases the current/pre
		//separation by one.
		current = current->next;
		separation--;
		
		//Update the elf counter and (if necessary) advance the pre pointer
		numElves--;
		while (separation < numElves/2 - 1)
		{
			pre = pre->next;
			separation++;
		}
	}
	
	//Print the answer
	printf("The final elf is #%ld with %ld presents\n", current->position,
	                                                    current->presents);
	
	//Free the last elf('s soul)
	free(current);
	
	return EXIT_SUCCESS;
}
