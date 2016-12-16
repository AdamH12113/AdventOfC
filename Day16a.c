//Day16a.c
//
//The sixteenth challenge is to use a modified dragon curve algorithm to create
//pseudo-random data, then compute a checksum of that data. The generation
//algorithm works like this:
//
//1. Copy the existing data.
//2. Reverse the order of the characters.
//3. Append a zero to the original data.
//4. Append the reversed data to the result of step 3.
//
//This process repeats until the desired amount of data has been generated. The
//algorithm may generate more data than needed, in which case only the necessary
//data will be used for the checksum.
//
//The checksum algorithm converts every pair of characters into a single
//character -- 11 and 00 become 1, and 01 and 10 become 0. This halves the
//length of the data. The checksum process is repeated until the result is an
//odd number of characters. This final string is the checksum.
//
//Question 1: If we need 272 characters of data, what is the checksum given the
//provided input string?
//
//To solve this puzzle, we can do some straightforward array manipulation. The
//checksum operation is basically an XNOR.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>


#define MAX_DATA_LENGTH  600


int main(int argc, char **argv)
{
	//We need some arrays to store the characters. We'll never need more than
	//545, but let's round up to 600 just to be on the safe side.
	char data[MAX_DATA_LENGTH];
	size_t numChars, dataNeeded;
	int c;
	
	//Let's take two arguments this time to make using the test data easier. The
	//first will be the input data and the second will be the number of
	//characters needed.
	if (argc != 3)
	{
		fprintf(stderr, "Usage:\n\tDay16 <input data> <characters needed>\n\n");
		return EXIT_FAILURE;
	}
	
	//Figure out the size of the initial data. We're assuming good input here;
	//in real life we'd need error checking.
	numChars = strlen(argv[1]);
	dataNeeded = atoi(argv[2]);
	
	//Copy the initial data into the data array, converting to numerical values
	//in the process.
	for (c = 0; c < numChars; c++)
	{
		data[c] = argv[1][c] - '0';
	}

	//Now we can do the dragon algorithm until we have enough data
	while (numChars < dataNeeded)
	{
		//Append a zero to the data
		data[numChars] = 0;
		
		//Copy the starting data to the end in reverse with values inverted
		for (c = 0; c < numChars; c++)
		{
			//Let's unpack this. The starting data goes from 0 to numChars-1.
			//The zero goes at numChars. The reversed data goes from numChars+1
			//to 2*numChars. The loop counter goes from 0 to numChars-1. Here's
			//a table:
			//    Position  Source                   Destination
			//--------------------------------------------------------------
			//    First     numChars-1 - 0           numChars+1 + 0
			//    Middle    numChars-1 - c           numChars+1 + c
			//    Last      numChars-1 - numChars-1  numChars+1 + numChars-1
			//          or: 0                        2*numChars
			data[(numChars+1) + c] = 1 - data[(numChars-1) - c];
		}

		//Adjust the character counter
		numChars = 2*numChars + 1;
	}
	
	//Ignore any unneeded characters
	numChars = dataNeeded;
	
	//Calculate the checksum. We can shrink the data in the existing array until
	//the number of characters is odd. This is another key use of modulus
	//division.
	while (numChars % 2 != 1)
	{
		for (c = 0; c < numChars; c += 2)
		{
			if (data[c] == data[c+1])
				data[c/2] = 1;
			else
				data[c/2] = 0;
		}
		
		//Now we have half as many characters
		numChars /= 2;
	}
	
	//Print the checksum, converting the data back to printable characters in
	//the process.
	printf("Checksum: ");
	for (c = 0; c < numChars; c++)
	{
		printf("%c", data[c] + '0');
	}
	printf("\n");
	
	return EXIT_SUCCESS;
}
