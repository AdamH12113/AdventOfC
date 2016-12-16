//Day10a.c
//
//The tenth challenge involves numbered microchips being moved around by
//numbered robots. When a bot receives two microchips, it compares the chip
//numbers to see which is higher. The bot gives its high chip to one bot or
//output bin and its low chip to another bot or output bin. Bots only do the
//comparison and transfer when they have two chips.
//
//Our input is a series of lines, each of which contains one of two types of
//instruction. The first type gives a single chip (numbered X) to a bot
//(numbered Y):
//
//    value X goes to bot Y
//
//The second type tells bot X what to do with its low and high chips:
//
//    bot X gives low to bot Y and high to bot Z
//    bot X gives low to output Y and high to bot Z
//    bot X gives low to output Y and high to output Z
//
//Question 1: What is the number of the bot that compares chip 61 with chip 17?
//
//To solve this puzzle, we can model the instructions as connections between the
//bots. The question doesn't mention the output bins, but they might be
//important for part B, so we should model them too.
//
//Each bot has four attributes -- two chips and two destinations. The bots are
//not in order in the input file, so we'll have to figure out the list sizes in
//order to allocate memory.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

//Bot information
typedef struct
{
	int chip1, chip2;
	sBot *lowDest, *highDest;
}


int main(int argc, char **argv)
{
	FILE *inFile;
	
	//The usual command line argument check and input file opening
	if (argc != 2)
	{
		fprintf(stderr, "Usage:\n\tDay10 <input filename>\n\n");
		return EXIT_FAILURE;
	}

	inFile = fopen(argv[1], "r");
	if (inFile == NULL)
	{
		fprintf(stderr, "Error opening file: %s\n\n", strerror(errno));
		return EXIT_FAILURE;
	}

	//Read the lines one at a time and count the letters
	while (fgets(line, MESSAGE_LEN+1, inFile) != NULL)
	{

	}
	
	//Close the file as soon as we're done with it
	fclose(inFile);
	

	
	
	return EXIT_SUCCESS;
}
