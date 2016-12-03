//Day3a.c
//
//The third challenge is to determine whether groups of three numbers could be
//the side lengths of a triangle. For this to be possible, the sum of any two
//sides must be larger than the remaining side. We have to count the number of
//possible triangles in the input.
//
//Our input is a series of lines. Each line has three numbers separated by
//spaces. These are the (hypothetical) side lengths. Each line has exactly 16
//characters (including the newline at the end, so we don't need to do any fancy
//memory allocation.
//
//Question 1: How many of the lines describe possible triangles?
//
//To solve this puzzle, we have to test each combination of sides. The title of
//this puzzle is "Squares With Three Sides", so question 2 will almost certainly
//reveal that we're dealing with a different shape. But this problem is simple
//enough that there's not much point in trying to plan ahead.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define LINE_LENGTH 16


int main(int argc, char **argv)
{
	//When allocating space for strings, always leave room for the terminating
	//null character!
	char line[LINE_LENGTH+1];
	char *next;
	FILE *inFile;
	long s1, s2, s3;
	int numTriangles = 0;
	
	//The usual command line argument check and input file opening
	if (argc != 2)
	{
		fprintf(stderr, "Usage:\n\tDay2 <input filename>\n\n");
		return EXIT_FAILURE;
	}

	inFile = fopen(argv[1], "r");
	if (inFile == NULL)
	{
		fprintf(stderr, "Error opening file: %s\n\n", strerror(errno));
		return EXIT_FAILURE;
	}

	//Read the file one line at a time. fgets() automatically stops after a
	//newline, but we won't rely on that behavior here. Note that the length
	//passed to fgets() includes the terminating null character.
	while (fgets(line, LINE_LENGTH+1, inFile) != NULL)
	{
		//We could use strtok() and atoi() like on Day 1, but since we're only
		//dealing with numbers we can use strtol() instead. strtol() takes in a
		//pointer to a string, a reference to a pointer, and a number base.
		//Unlike in C++, there's no special syntax for refernces -- we just
		//pass in a pointer to a variable. In this case, that's a pointer to
		//a pointer. strtol() returns the converted number and modifies the
		//referenced pointer to point to the remaining part of the string. The
		//string is not modified. To get the next number, we pass in the
		//modified pointer. strtol() returns 0 on failure. If 0 were a valid
		//value, we could check errno instead.
		s1 = strtol(line, &next, 10);
		s2 = strtol(next, &next, 10);
		s3 = strtol(next, &next, 10);
		if (s1 == 0 || s2 == 0 || s3 == 0)
		{
			fprintf(stderr, "Error parsing numbers: %s\n\n", strerror(errno));
			fprintf(stderr, "Line: %s\n", line);
			fclose(inFile);
			return EXIT_FAILURE;
		}
		
		//Check for a possible triangle and increment the count if necessary
		if (s1 + s2 > s3 && s2 + s3 > s1 && s3 + s1 > s2)
			numTriangles++;
	}
	
	//Close the file as soon as we're done with it
	fclose(inFile);
	
	//Print the number of triangles
	printf("Number of possible triangles: %d\n", numTriangles);
	
	return EXIT_SUCCESS;
}
