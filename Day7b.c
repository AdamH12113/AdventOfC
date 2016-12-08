//Day7b.c
//
//Question 2: Matching addresses now need an ABA sequence outside square
//brackets and a corresponding BAB sequence inside square brackets. For example,
//if we see "xyx" outside square brackets, then seeing "yxy" inside square
//brackets would count as a match. Note that there may be multiple ABA sequences
//in the address, and that an ABA sequence may come after its BAB sequence. How
//many matching addresses are there?
//
//This just got nasty. Instead of just looking at the current four characters,
//now we have to save all of the ABA and BAB sequences we find for later
//comparison. So much for a simple exercise! On the bright side, the
//slightly-generic shift register is still useful.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

//The shift register doesn't change
struct sShiftRegister
{
	char *data;
	size_t size;
};

typedef struct sShiftRegister *SR_Handle;

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

char SR_Shift(SR_Handle sr, char in)
{
	size_t c;
	char out;
	
	if (sr == NULL)
		return '\0';
	
	out = sr->data[0];
	
	for (c = 0; c < sr->size - 1; c++)
	{
		sr->data[c] = sr->data[c+1];
	}

	sr->data[sr->size - 1] = in;
	return out;
}

char SR_Read(SR_Handle sr, size_t element)
{
	if (sr == NULL)
		return '\0';
		
	if (element > sr->size)
		return '\0';
		
	return sr->data[element];
}

void SR_Destruct(SR_Handle sr)
{
	if (sr != NULL)
		free(sr->data);
	
	free(sr);
}


//We won't know which sequences match until the line is over, so we have to save
//them all. We'll implement this as a structure with a two-character array and a
//boolean tag that indicates whether the sequence is ABA (outside brackets) or
//BAB (inside brackets). We only need two characters because the sequence is
//symmetrical. We'll create an array of these structures and grow it dynamically
//like we did on Day 4.
typedef struct
{
	char letters[2];
	bool isBAB;
} sABA;


int main(int argc, char **argv)
{
	//We don't need to save match and fail status anymore since we determine
	//those at the end of the line. We do need a list pointer, though.
	sABA *aba;
	FILE *inFile;
	SR_Handle sr;
	int next, addrCount, numABA, bufSize, s1, s2;
	bool inBrackets;
	
	//Start the ABA buffer with four sequences available
	bufSize = 4;
	aba = malloc(bufSize * sizeof(sABA));
	memset(aba, 0, bufSize * sizeof(sABA));
	
	//The new shift register only needs three elements
	sr = SR_Construct(3);
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
	numABA = 0;
	inBrackets = false;
	
	//Read characters one at a time
	while ((next = fgetc(inFile)) != EOF)
	{
		//Shift the new character into the register
		SR_Shift(sr, next);
	
		//A newline is still our trigger to evaluate the address
		if (next == '\n')
		{
			//Instead of checking for matches on the fly, we now do pairwise
			//comparisons between every stored ABA sequence at the end of the
			//line.
			for (s1 = 0; s1 < numABA; s1++)
			{
				for (s2 = s1 + 1; s2 < numABA; s2++)
				{
					//Don't forget, the letters still have to be in reverse
					//order even though we have the boolean flag.
					if (aba[s1].letters[0] == aba[s2].letters[1] &&
					    aba[s1].letters[1] == aba[s2].letters[0] &&
						aba[s1].isBAB != aba[s2].isBAB)
					{
						addrCount++;
						
						//Once the comparison passes, we don't want to increment
						//the count anymore. We could use a boolean variable
						//here and increment the counter later, but instead I'm
						//going to show a valid use of
						//
						//    THE DREADED GOTO
						//
						//which is breaking out of nested loops. BEHOLD ITS
						//TERRIBLE SPLENDOR:
						goto ABA_CHECK_DONE;
					}
				}
			}

			//Here's the goto label. Don't feel bad -- at the assembly level,
			//it's ALL gotos. ;-)
			ABA_CHECK_DONE:
		
			//Strangely, the variable reset gets simpler
			numABA = 0;
		}
		
		//Square brackets work the same way
		if (next == '[')
			inBrackets = true;
		if (next == ']')
			inBrackets = false;
		
		//Check for an ABA pattern in the shift register. Special characters can
		//be in the middle of an ABA sequence, so we need to check for that.
		if (SR_Read(sr, 0) != SR_Read(sr, 1) &&    //Letters aren't all the same
		    SR_Read(sr, 0) == SR_Read(sr, 2) &&    //Outer letters match
		    SR_Read(sr, 1) != '[' &&               //No special characters
			SR_Read(sr, 1) != ']' &&
			SR_Read(sr, 1) != '\n')
		{
			//First, make sure the buffer is big enough
			numABA++;
			if (numABA > bufSize)
			{
				bufSize *= 2;
				aba = realloc(aba, bufSize * sizeof(sABA));
				if (aba == NULL)
				{
					fprintf(stderr, "Error reallocating: %s", strerror(errno));
				}
			}
			
			//Now we can write the new sequence
			aba[numABA-1].letters[0] = SR_Read(sr, 0);
			aba[numABA-1].letters[1] = SR_Read(sr, 1);
			
			//ABA vs. BAB is determined by whether we're in brackets
			aba[numABA-1].isBAB = inBrackets;
		}
	}
	
	//Close the file and destroy the shift register
	fclose(inFile);
	SR_Destruct(sr);
		
	//Print the number of matches
	printf("Matching addresses: %d\n", addrCount);	
	
	return EXIT_SUCCESS;
}
