//Day11a.c
//
//The eleventh challenge is to move a collection of generators and microchips to
//the top floor of a facility. Each microchip is compatible with one generator.
//The generators are radioactive, and will destroy incompatible microchips if
//they end up on the same floor. Microchips are safe from damage when connected
//to their compatible generator (i.e. when they're on the same floor).
//
//The four floors of the facility are connected by an elevator, which moves one
//floor at a time. To move, the elevator must hold either one or two components
//in any combination. The elevator starts on the first floor.
//
//Our input is a series of lines, each of which lists the components on one
//floor. 
//
//Question 1: What is the minimum number of steps required to bring all of the
//components to the fourth floor?
//
//This is a kind of river crossing puzzle. To solve it, we'll have to rely on
//the fact that the number of valid moves is limited, and many valid moves are
//equivalent. For example, if we have three matching microchip/generator pairs
//on our current floor, our valid moves could be to move one pair or move two
//microchips. Which pair or which microchips doesn't matter -- there's no
//difference between them! Dealing with two choices is much easier than dealing
//with six.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


//Instead of mucking about with text parsing, let's enter the input state
//manually so we can concentrate on the task at hand. My input has fives types
//of power -- strontium (S), plutonium (P), thulium (T), ruthenium (R), and
//curium (C). There's also the elevator (E), which starts on floor 1. The
//starting location of each generator and microchip is:
//
//               
//    Floor 4    .   .   .   .   .   .   .   .   .   .   . 
//    Floor 3    .   .   .   .   .   .   TM  .   .   .   .
//    Floor 2    .   .   .   .   .   TG  .   RG  RM  CG  CM
//    Floor 1    E   SG  SM  PG  PM  .   .   .   .   .   .



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
		fprintf(stderr, "Usage:\n\tDay11 <input filename>\n\n");
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
