//Day20.c
//
//The twentieth challenge is to find valid 32-bit values ("addresses") that
//aren't in a list of blocked values.
//
//Our input is the list, which consists of a series of lines. Each line contains
//one blocked address range in the form:
//
//    X-Y
//
//where X and Y are both decimal integers. The ranges may overlap.
//
//Question 1: What is the smallest address that isn't blocked?
//Question 2: How many addresses are unblocked?
//
//To solve this puzzle, we (obviously) need a way to determine whether a
//specific address is blocked. More importantly, it needs to be efficient --
//with four billion possible addresses, an array of booleans or repeated
//guess-and-check parsing would take a lot of resources.
//
//This is the same kind of problem that malloc() has to solve when it looks for
//a block of free memory. We can use the same solution here -- a linked list
//containing ranges of blocked and free addresses. We'll construct the list one
//node at a time, starting with a single free range covering (0) - (2^32 - 1).
//Whenever we add a blocked range, we'll split and/or combine the nodes that
//contain it into new nodes. Once the list is constructed, all we have to do is
//walk it to find the first valid node. There are only about a thousand blocked
//ranges, so this shouldn't take too long.


//We need inttypes.h to get the format specifier constant for uint32_t. That
//header also #includes stdint.h.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <inttypes.h>


//Node definition for the range list. We need a starting and ending address
//(from the input), a boolean to mark which ranges are blocked, and a pointer
//to the next node. Since the addresses are exactly 32 bits, we'll use the
//corresponding stdint.h type. To simplify the code, we'll make this a doubly-
//linked list.
typedef struct sRange
{
	uint32_t start;
	uint32_t end;
	bool blocked;
	struct sRange *next;
	struct sRange *prev;
} sRange;


sRange *Insert_Node(sRange *before);
void Delete_Node(sRange *node);
void Delete_Intermediate_Nodes(sRange *keepLow, sRange *keepHigh);


int main(int argc, char **argv)
{
	//The maximum line length is two ten-digit numbers, a hyphen, and a newline.
	//Add one for the null terminator and we've got a buffer.
	FILE *inFile;
	char line[10+1+10+1+1];
	sRange *list, *low, *high;
	uint32_t first, second, total;
	
	//The usual command line argument check and input file opening
	if (argc != 2)
	{
		fprintf(stderr, "Usage:\n\tDay20 <input filename>\n\n");
		return EXIT_FAILURE;
	}

	inFile = fopen(argv[1], "r");
	if (inFile == NULL)
	{
		fprintf(stderr, "Error opening file: %s\n\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
	//Create the first node, which contains the entire address range
	list = malloc(sizeof(sRange));
	if (list == NULL)
	{
		fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
		fclose(inFile);
		return EXIT_FAILURE;
	}
	
	//We'll use a null pointer to indicate the ends of the list. I'm using
	//hexadecimal here because it's easiest to write the maximum uint32_t value
	//in hex.
	list->start = 0x00000000;
	list->end = 0xffffffff;
	list->blocked = false;
	list->next = NULL;
	list->prev = NULL;
	
	//Read the input file one line at a time
	while (fgets(line, sizeof(line), inFile) != NULL)
	{
		//Since we need to read values that may be beyond the range of a signed
		//long integer, we can't use strtol(). This seems like a good time to
		//break out the scanf() family, which works well with fixed-format data.
		//On my PC a uint32_t is the same as an unsigned int, but instead of
		//assuming that let's use the weird format specifier constant from
		//inttypes.h. After all, there are plenty of machines where an int isn't
		//32 bits! (Not sure how many you'd be using scanf() on, but...)
		sscanf(line, "%" PRIu32 "-%" PRIu32, &first, &second);
		
		//Find the first node whose range contains the first address
		low = list;
		while (first < low->start || first > low->end)
		{
			low = low->next;
		}
		
		//Find the first node whose range contains the high address
		high = low;
		while (second < high->start || second > high->end)
		{
			high = high->next;
		}
		
		//Conceptually, what we need to do is simple -- insert the new blocked
		//range into the middle of the existing list. Unfortunately, we have to
		//handle several variations based on how existing nodes cover the
		//address range. The new range could correspond to:
		//
		//1. One full node, exactly
		//2. Multiple nodes, exactly
		//3. Part of one node
		//4. Part of two nodes
		//5. Part of one node and all of one or more nodes
		//6. Part of two nodes and all of one or more nodes
		//
		//What we have to do for each of these is:
		//
		//1. Set the node flag to blocked.
		//2. Replace the multiple nodes with one blocked node
		//3. Split the node into two or three new nodes (one blocked)
		//4. Create a third node in between the two existing nodes and update
		//   the ranges of the boundary nodes.
		//5. Split the partial node and delete the other nodes
		//6. Same as #4, but also delete the intermediate nodes
		//
		//Looking at these, we can see a few common operations -- we always
		//update the low node, and we always delete intermediate nodes.
		//This will help us simplify the algorithm.
		Delete_Intermediate_Nodes(low, high);
		if (low->start == first && high->end == second)
		{
			printf("One\n", first, second);
			//One or more nodes represents the full address range and nothing
			//else, so we only need one node.
			if (low != high)
				Delete_Node(high);
			
			//Update the remaining node to cover the blocked address range and
			//set its blocked flag.
			low->end = second;
			low->blocked = true;
		} else if (low->start == first)
		{
			//When one node aligns with the low end of the range, we only need
			//two nodes. If we only have one, we'll need to add another.
			if (low == high)
			{
				high = Insert_Node(low);
				high->end = low->end;
				high->blocked = low->blocked;
			}
			
			//Update the node ranges and flags
			low->end = second;
			low->blocked = true;
			high->start = second + 1;
		} else if (high->end == second)
		{
			//Same as above, but this time a node aligned with the high end of
			//the range. If we need it, a new node goes before that one.
			if (low == high)
			{
				low = Insert_Node(low->prev);
				low->start = high->start;
				low->blocked = high->blocked;
			}
			
			//Update the node ranges and flags
			low->end = first - 1;
			high->start = first;
			high->blocked = true;
		} else
		{
			//No node aligned with an end of the range, so we need a total of
			//three nodes.
			if (low == high)
			{
				high = Insert_Node(low);
				high->end = low->end;
				high->blocked = low->blocked;
				Insert_Node(low);
			} else if (low->next == high)
			{
				Insert_Node(low);
			}
			
			//Update the node ranges and flags
			low->end = first - 1;
			low->next->start = first;
			low->next->end = second;
			low->next->blocked = true;
			high->start = second + 1;
		}
	}
	
	//Walk the list to find the first unblocked address
	low = list;
	while (low->blocked)
	{
		low = low->next;
	}
	
	//Print the first valid address using the weird inttypes.h format specifier
	//constant.
	printf("The first valid address is %" PRIu32 "\n", low->start);
	
	//Continue walking the list to count the total number of unblocked addresses
	total = 0;
	do
	{
		if (low->blocked == false)
		{
			//The address range is inclusive, so add one after subtracting the
			//start and end addresses.
			total += (low->end - low->start) + 1;
		}
		
		low = low->next;
	} while (low != NULL);
	
	//Print the total
	printf("The total number of valid addresses is %" PRIu32 "\n", total);
	
	return EXIT_SUCCESS;
}


//Helper function for inserting a node after another node. Allocates memory with
//malloc(). Returns a pointer to the new node.
sRange *Insert_Node(sRange *before)
{
	sRange *new;
	
	//Allocate memory for the new node
	new = malloc(sizeof(sRange));
	if (new == NULL)
	{
		//In a real program it might be better to keep the error handling in
		//main(), but doing it here makes the main() code cleaner.
		fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	//Initialize the new node's pointers. Its other state must be set by the
	//calling function.
	new->next = before->next;
	new->prev = before;

	//Update the pointers of the adjacent nodes to insert the new node. Don't
	//forget to handle the end of the list!
	before->next = new;
	if (new->next != NULL)
		new->next->prev = new;
	
	return new;
}

//Helper function for deleting a node. Frees the node's memory.
void Delete_Node(sRange *node)
{
		//Update the pointers of the adjacent nodes to skip the deleted node.
		//Don't forget to handle the ends of the list!
		if (node->next != NULL)
			node->next->prev = node->prev;
		
		if (node->prev != NULL)
			node->prev->next = node->next;
		
		//Free the node's memory to delete it
		free(node);
}

//Helper function for deleting intermediate nodes within a blocked range
void Delete_Intermediate_Nodes(sRange *keepLow, sRange *keepHigh)
{
	//If there are no intermediate nodes, don't do anything
	if (keepLow == keepHigh || keepLow->next == keepHigh)
		return;

	//Delete intermediate nodes until they're gone
	while (keepLow->next != keepHigh)
	{
		Delete_Node(keepLow->next);
	}
}
