//Day7a.c
//
//The seventh challenge is to do pattern matching on a series of "IPv7"
//addresses to find which ones contain ABBA sequences -- a pair of two different
//letters followed by the same pair in reverse order. For example, ghhg, ewwe,
//and kxxk are all ABBA sequence, but ghij, ewew, and bbbb are not. However, the
//address must not have any ABBA sequences contained within square brackets.
//
//Our input is a series of line, each of which contains one address of arbitrary
//length. Addresses may have multiple sets of letters in square brackets.
//
//Question 1: How many addresses contain ABBA sequences only outside of square
//brackets?
//
//To solve this puzzle, we need to iterate over the strings and compare multiple
//characters at once. One approach would be to read in the whole line at once,
//allocating more memory as necessary. If performance were a priority, that
//would be a good idea. But if we want to conserve memory instead, we can take
//advantage of the fact that we only need to store four characters at a time:
//
//                       Newest character
//                       |
//        +----+----+----+--- Possible ABBA sequence
//        |    |    |    |
//        v    v    v    v     
//  ----+----+----+----+----+----+
//  ... | C-3| C-2| C-1| C0 | ...
//  ----+----+----+----+----+----+
//
//For this, we need to implement a shift register. A shift register is a FIFO
//data structure that allows access to all of its data in parallel:
//
//                    +----+----+----+----+
// Shift data out <-- | C-3| C-2| C-1| C0 | <-- Shift data in
//                    +----+----+----+----+
//
//Just for fun, let's use some simple object-oriented techniques to make
//something more reusable.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

//Internal structure. User code will only interact with this through functions
//and an opaque pointer type.
struct sShiftRegister
{
	char *data;
	size_t size;
};

//Opaque type that gets passed to the functions. This sort of thing is sometimes
//called a "context".
typedef struct sShiftRegister *SR_Handle;

//"Constructor". Allocates memory for an sShiftRegister structure along with the
//actual data. The register data is initialized to all zeros.
SR_Handle SR_Construct(size_t numElements)
{
	SR_Handle newSR;

	if (numElements == 0)
		return NULL;

	newSR = malloc(sizeof(struct sShiftRegister));
	if (newSR == NULL)
	{
		return NULL;
	} else
	{
		newSR->size = numElements;
		newSR->data = malloc(numElements);
		if (newSR->data == NULL)
		{
			free(newSR);
			return NULL;
		} else
		{
			memset(newSR->data, 0, numElements);
			return newSR;
		}
	}
}

//Perform a shift operation. This returns a character from the start of the data
//array, shifts the other characters to make room at the end, then writes the
//new character to the end.
char SR_Shift(SR_Handle sr, char in)
{
	size_t c;
	char out;
	
	//We're not being very defensive in these exercises, so this seems like a
	//reasonable failure mode.
	if (sr == NULL)
		return '\0';
	
	//Save the output character
	out = sr->data[0];
	
	//Shift all the other characters forward one space
	for (c = 0; c < sr->size - 1; c++)
	{
		sr->data[c] = sr->data[c+1];
	}

	//Write the new character to the end of the data array and return the
	//output character.
	sr->data[sr->size - 1] = in;
	return out;
}

//Read an arbitrary element of the shift register
char SR_Read(SR_Handle sr, size_t element)
{
	//Basic error handling. Theoretically we could combine the null check and
	//the size check into a single if statement:
	//
	//if (sr == NULL || element > sr->size)
	//
	//C's short-circuiting behavior should guarantee that sr is never
	//dereferenced if it's null. But getting sloppy about null pointer
	//checks can cause trouble. See, for instance:
	//https://lwn.net/Articles/342330/
	if (sr == NULL)
		return '\0';
		
	if (element > sr->size)
		return '\0';
		
	//Read and return the data element
	return sr->data[element];
}

//"Destructor". Frees allocated memory for both the data and the structure.
void SR_Destruct(SR_Handle sr)
{
	if (sr != NULL)
		free(sr->data);
	
	free(sr);
}


//With our new shift register type defined, we can move on to main()
int main(int argc, char **argv)
{
	//For lack of better terms, I'll call an ABBA sequence outside of square
	//brackets a "match" and an ABBA sequence inside square brackets a "fail".
	FILE *inFile;
	SR_Handle sr;
	int next, addrCount;
	bool match, fail, inBrackets;
	
	//Create a shift register with four elements
	sr = SR_Construct(4);
	if (sr == NULL)
	{
		fprintf(stderr, "Error creating shift register\n\n");
		return EXIT_FAILURE;
	}
	
	//The usual command line argument check and input file opening
	if (argc != 2)
	{
		fprintf(stderr, "Usage:\n\tDay7 <input filename>\n\n");
		return EXIT_FAILURE;
	}

	inFile = fopen(argv[1], "r");
	if (inFile == NULL)
	{
		fprintf(stderr, "Error opening file: %s\n\n", strerror(errno));
		return EXIT_FAILURE;
	}

	//Set up for the main loop
	addrCount = 0;
	match = false;
	fail = false;
	inBrackets = false;
	
	//Read characters one at a time
	while ((next = fgetc(inFile)) != EOF)
	{
		//Shift the new character into the register
		SR_Shift(sr, next);
	
		//Determine the final status of the address and reset the booleans on a
		//newline.
		if (next == '\n')
		{
			if (match && !fail)
				addrCount++;
			
			match = false;
			fail = false;
		}
		
		//A square bracket changes whether we're inside brackets
		if (next == '[')
			inBrackets = true;
		if (next == ']')
			inBrackets = false;
			
		//Check for an ABBA pattern in the shift register. We could skip this
		//when we see a special character, but they can't form ABBA patterns, so
		//it's not worth the trouble.
		if (SR_Read(sr, 0) != SR_Read(sr, 1) &&    //Letters aren't the same
		    SR_Read(sr, 0) == SR_Read(sr, 3) &&    //Outer letters match
		    SR_Read(sr, 1) == SR_Read(sr, 2))      //Inner letters match
		{
			//This could be a match or a fail depending on whether we're
			//currently in square brackets.
			if (inBrackets)
				fail = true;
			else
				match = true;
		}
	}
	
	//Close the file and destroy the shift register
	fclose(inFile);
	SR_Destruct(sr);
		
	//Print the number of matches
	printf("Matching addresses: %d\n", addrCount);	
	
	return EXIT_SUCCESS;
}
