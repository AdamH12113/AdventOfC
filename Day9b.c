//Day9b.c
//
//Question 2: If markers within repeated sequences are expanded (such that the
//expanded sequence gets repeated), how many characters are in the output?
//
//The Advent of Code page hints that we might not have enough memory to
//decompress the file, so we'll have to find another way to get the length. We
//don't actually have to write the output -- we could remove the string from
//our string type and just track the lengths -- but that might take a long time
//to run. Instead, let's do something overly clever -- recursion!
//
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

//Defining a structure to hold the marker information and a function to read it
//from the string will simplify things. We never need to modify the values once
//they're read, and they're fairly small, so we can pass this struct by value.
typedef struct
{
	int numChars;       //Number of characters to read
	int numRepeats;     //Number of times to repeat
	size_t markerSize;  //Number of characters in the marker itself
	long long sum;      //Number of characters that would be output
} sMarker;

//Read the parenthesized marker sequence and return its information as an
//sMarker structure.
sMarker Read_Marker(char *start)
{
	sMarker retVal;
	
	//We need to get the length of the marker. There's actually a standard
	//library function that can do this -- strcspn(). It returns the number of
	//characters at the start of the string that aren't in a specified list of
	//values. Here, we're looking for the end of the marker, which is the close
	//parenthesis.
	retVal.markerSize = strcspn(start, ")") + 1;
	
	//Skip the opening parenthesis, then call strtol() to get the number of
	//characters to repeat. We'll have strtol() update the string pointer while
	//we're at it.
	start++;
	retVal.numChars = strtol(start, &start, 10);
	
	//Skip the 'x', then get the number of repeats
	start++;
	retVal.numRepeats = strtol(start, NULL, 10);
	
	return retVal;
}


int main(int argc, char **argv)
{
	//Reading the whole input into memory will make this much easier. As for the
	//output, we have to assume that it might be in the multi-gigabyte range, so
	//a 64-bit variable is appropriate.
	FILE *inFile;
	sMarker marker;
	size_t inSize;
	char *in, *next;
	long long sum;
	
	//The usual command line argument check
	if (argc != 2)
	{
		fprintf(stderr, "Usage:\n\tDay9 <input filename>\n\n");
		return EXIT_FAILURE;
	}

	inFile = fopen(argv[1], "r");
	if (inFile == NULL)
	{
		fprintf(stderr, "Error opening file: %s\n\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
	//Get the file size and allocate memory. Don't forget to leave room for the
	//null terminator!
	fseek(inFile, 0, SEEK_END);
	inSize = (size_t)ftell(inFile);
	rewind(inFile);
	
	//Allocate memory for the input data. We need an extra spot for the null
	//terminator, but we don't need the newline at the end. The two cancel out.
	in = malloc(inSize + 1 - 1);
	if (in == NULL)
	{
		fprintf(stderr, "Error allocating memory: %s\n\n", strerror(errno));
		fclose(inFile);
		return EXIT_FAILURE;
	}

	//Read the file into memory, dropping the newline at the end
	fgets(in, inSize + 1 - 1, inFile);
	
	//Close the file as soon as we're done with it
	fclose(inFile);

	//Main loop. Call the recursive helper function every time we encounter a
	//marker sequence.
	next = in;
	sum = 0;
	while (*next != '\0')
	{
		if (*next == '(')
		{
			marker = Read_Marker(next);
		} else
		{
			sum++;
		}
		
		//Advance to the next character
		next++;
	}

	//Print the total number of characters
	printf("Number of characters: %lld\n", sum);
	
	return EXIT_SUCCESS;
}


//When we encounter a marker sequence, we need to do three things:
//
//1. Parse the marker and get its information.
//2. Look for nested markers. If we find any, recursively call this function.
//3. Add up the number of characters in the marked string (plus the result of
//   any nested markers) and return it.
sMarker Handle_Marker(char *markerStart)
{
	sMarker this;
	int numChars = 0;
	
	//Parse the marker using our handy helper function
	this = Read_Marker(markerStart);
	
	//Skip ahead to the marked characters
	markerStart += this.markerSize;
	
	//
	
	
	
	
	
	
	
	
	

