//Day4a.c
//
//The fourth challenge is to compute special checksums for input strings, then
//compare them to provided checksums. The checksum consists of the five most
//common letters in the string, starting with the most common and ending with
//the least. Ties are broken alphabetically.
//
//Our input is a series of lines. Each line has one string with its checksum.
//The first part of the string consists of groups of letters separated by
//hyphens. The number of groups varies. After the last group, there's another
//hyphen followed by a number called the sector ID. After that comes the
//provided checksum in square brackets followed by a newline. Here are some
//examples:
//
//    hqcfqwydw-fbqijys-whqii-huiuqhsx-660[qhiwf]  
//    kzeed-idj-xmnuunsl-593[uazmr]
//    bnmrtldq-fqzcd-ahngzyzqcntr-atmmx-dmfhmddqhmf-989[mdqfh]
//
//Question 1: What is the sum of the sector IDs of the strings with correct
//checksums?
//
//To solve this puzzle, we just have to compute and compare the checksums. We
//could read lines into a character array, then iterate over the array. We could
//use strtok() to help us break up the lines. If we had a regular expression
//library, we could use it to help parse the strings. Someone's probably got a
//Perl one-liner for this. :-) It's not a hard problem. But since the strings
//don't have a fixed length, let's process them one character at a time. This
//lets us handle arbitrarily long lines. They're all less than 80 characters in
//the actual input, but it's good practice.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

//We all know there are 26 letters, but it's still bad practice to put random
//constants in your code.
#define NUM_LETTERS  26
#define CHKSUM_LEN   5


void Calc_Chksum(int *counts, char *chksum, int numCounts, int chksumSize);

int main(int argc, char **argv)
{
	FILE *inFile;
	int counts[NUM_LETTERS];
	char calcChksum[CHKSUM_LEN];
	char readChksum[CHKSUM_LEN];
	//We could use a pointer or an integer to keep track of how many checksum
	//characters we've read. Let's use a pointer to illustrate some common
	//pointer syntax.
	char *chksumChar;
	bool inChksum;
	int nextChar;
	//Why use a long here instead of an int? Int is only guaranteed to be at
	//least 16 bits. There are almost a thousand lines of input and the sector
	//IDs are three-digit numbers, so it would be quite possible to get a sum
	//that's larger than 32767. Using a long guarantees we won't overflow. A lot
	//of x86 programmers never worry about this stuff, but in the embedded world
	//it's easy to get into trouble. That's why we normally use the stdint.h
	//types like uint32_t. Likewise, almost everyone assumes a char is 8 bits,
	//but on old mainframes or new DSPs that's not necessarily true...
	long idSum = 0;
	long id;

	//The usual command line argument check and input file opening
	if (argc != 2)
	{
		fprintf(stderr, "Usage:\n\tDay4 <input filename>\n\n");
		return EXIT_FAILURE;
	}

	inFile = fopen(argv[1], "r");
	if (inFile == NULL)
	{
		fprintf(stderr, "Error opening file: %s\n\n", strerror(errno));
		return EXIT_FAILURE;
	}

	//We need to do four tasks for each line:
	//
	//1. Count up the letters
	//2. Extract the sector ID
	//3. Read the provided checksum
	//4. Compute the actual checksum
	//5. Compare the two checksums
	//
	//We'll pretend there's an initial newline character. This forces the loop
	//to initialize the variables first.
	nextChar = '\n';
	do
	{
		if (nextChar == '\n')
		{
			//If the character is a newline, we're (obviously) about to start a
			//new line. Clear the counts and (for easier debugging) reset the
			//checksum and sector ID.
			memset(counts, 0, NUM_LETTERS * sizeof(int));
			memset(calcChksum, '\0', CHKSUM_LEN * sizeof(char));
			memset(readChksum, '\0', CHKSUM_LEN * sizeof(char));
			chksumChar = readChksum;
			inChksum = false;
			id = 0;
		} else if (nextChar >= 'a' && nextChar <= 'z')
		{
			if (inChksum)
			{
				//If the letter is part of the checksum, save it. This syntax
				//is very common for iterating over an array. *pointer++ means
				//to access whatever the pointer points to now, then increment
				//the pointer so it points to the next element in the array.
				*chksumChar++ = (char)nextChar;
			} else
			{
				//If the next character is a letter in the string, add it to the
				//counts. This code breaks if the character set is EBCDIC. :-(
				counts[nextChar - 'a']++;
			}
		} else if (nextChar >= '0' && nextChar <= '9')
		{
			//If the next character is a number, add it to the sector ID
			id = 10*id + (nextChar - '0');
		} else if (nextChar == '[')
		{
			//If the character is an open square bracket, we're entering the
			//provided checksum and must handle letters differently.
			inChksum = true;
		} else if (nextChar == ']')
		{
			//We'll use the close bracket as a trigger to do the calculations
			Calc_Chksum(counts, calcChksum, NUM_LETTERS, CHKSUM_LEN);
			
			//Compare the checksums and add the ID to the total if they match.
			//We have to use strncmp() instead of strcmp() because we don't have
			//a terminating null character on the checksums. (Also, it's safer.)
			if (strncmp(calcChksum, readChksum, CHKSUM_LEN) == 0)
				idSum += id;
		}
	} while ((nextChar = fgetc(inFile)) != EOF);	
	
	//Close the file as soon as we're done with it
	fclose(inFile);
	
	//Print the sum of the sector IDs
	printf("Sum of sector IDs: %ld\n", idSum);
	
	return EXIT_SUCCESS;
}

//Calculate the checksum based on the letter counts
void Calc_Chksum(int *counts, char *chksum, int numCounts, int chksumSize)
{
	int max = 0, s = 0;
	int c;
	
	//Get the maximum count
	for (c = 0; c < numCounts; c++)
	{
		if (counts[c] > max)
			max = counts[c];
	}
	
	//Search for letters counts equal to the max in alphabetical order. Once all
	//the counts have been checked, decrement the max and try again. When all of
	//the checksum characters have been computed, stop.
	while (s < chksumSize)
	{
		//Breaking out of nested loops is always a little awkward. In this case,
		//it was easiest to check s in both the while loop and the for loop.
		for (c = 0; c < numCounts && s < chksumSize; c++)
		{
			//We never need to go back to the beginning of the checksum, so it's
			//okay to modify our only pointer.
			if (counts[c] == max)
			{
				*chksum++ = 'a' + c;
				s++;
			}
		}
		
		max--;
	}
}


