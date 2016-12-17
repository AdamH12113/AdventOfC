//Day9b.c
//
//Question 2: If markers within repeated sequences are expanded (such that the
//expanded sequence gets repeated), how many characters are in the output?
//
//The Advent of Code page hints that we might not have enough memory to
//decompress the file, so we'll have to find another way to get the length. We
//don't actually have to write the output -- we could remove the string from
//our string type and just track the lengths -- but that might take a long time
//to run. Instead, let's do something clever -- recursion! Recursion is a
//natural fit whenever you have to deal with data made up of units with the same
//structure as the whole. That's why functional programming languages love
//things like linked lists and binary trees -- each element of the data
//structure can act as a new instance of that data stucture!
//
//People are often wary of recursion because of stack usage, but it's not an
//issue for small problems like this. 


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


sMarker Count_Marker_Chars(char *place);
sMarker Read_Marker(char *start);


int main(int argc, char **argv)
{
	//Reading the whole input into memory will make this much easier. As for the
	//output, we have to assume that it might be in the multi-gigabyte range, so
	//a 64-bit variable is appropriate.
	FILE *inFile;
	sMarker whole;
	size_t inSize;
	char *in;
	
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
	
	//Get the file size and allocate memory. We'll drop the newline at the end.
	//Note that there's a nasty platform-specific quirk here -- on Windows, a
	//newline is a carriage return followed by a line feed -- two characters. On
	//Unix, a newline is just a line feed -- one character. If we assume the
	//wrong newline size, we either drop a character or add one, introducing an
	//off-by-one error into our result. Thus, we need to check what kind of
	//newline we have.
	fseek(inFile, -2, SEEK_END);
	if (fgetc(inFile) == '\n')
	{
		//We're on Windows -- drop an extra character. fgetc() reads both
		//newline characters, so it gets us back to the end of the file.
		inSize = (size_t)ftell(inFile) - 2;
	} else
	{
		//We're on Unix -- don't. fgetc() left us one character short of the
		//end of the file, so no adjustment is needed.
		inSize = (size_t)ftell(inFile);
	}
	rewind(inFile);
	
	//Allocate memory for the input data. We're going to add space for an extra
	//marker to help with the recursive function call later. Don't forget to
	//leave room for the null terminator!
	in = malloc(inSize + 9 + 1);
	if (in == NULL)
	{
		fprintf(stderr, "Error allocating memory: %s\n\n", strerror(errno));
		fclose(inFile);
		return EXIT_FAILURE;
	}

	//Create an x1 marker at the start of the string. This lets us treat the
	//whole string as a single marked sequence, which makes the processing more
	//convenient.
	sprintf(in, "(%dx1)", inSize);
	
	//Read the file into memory, minus the newline
	fgets(in + strlen(in), inSize+1, inFile);
	
	//Close the file as soon as we're done with it
	fclose(inFile);

	//Now we just call our marker processing function on the input. Since we
	//made the input a single marker sequence, only one call is necessary.
	whole = Count_Marker_Chars(in);
	
	//Print the total number of characters
	printf("Number of characters: %lld\n", whole.sum);
	
	return EXIT_SUCCESS;
}


//When we encounter a marker sequence, we need to do three things:
//
//1. Parse the marker and get its information.
//2. Look for nested markers. If we find any, recursively call this function.
//3. Add up the number of unmarked characters in the marked string (plus the
//   result of any nested markers), multiply that amount by the repeat count,
//   and return the result.
sMarker Count_Marker_Chars(char *place)
{
	sMarker this, sub;
	long long charCount;
	int c;
	
	//Parse the marker using our handy helper function
	this = Read_Marker(place);
	
	//Skip ahead to the marked characters
	place += this.markerSize;
	
	//Count normal characters while looking for a marker
	c = 0;
	charCount = 0;
	while (c < this.numChars)
	{
		if (place[c] == '(')
		{
			//When we find a marker, call the function recursively. Afterwards,
			//we'll need to update our character count and skip ahead past the
			//marked sequence.
			sub = Count_Marker_Chars(place + c);
			charCount += sub.sum;
			c += sub.markerSize + sub.numChars;
		} else
		{
			//For normal characters, just add one to the character count
			charCount++;
			c++;
		}
	}
	
	//Multiply by the number of repeats to get the size of the output sequence.
	this.sum = (long long)this.numRepeats * charCount;
	
	return this;
}

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
			
	
	
	
	
	
	

