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
//important for part B, so we should model them too. This gives us a binary tree
//with multiple roots. We can use a recursive algorithm to traverse the tree.
//
//Each bot has four attributes -- two chips and two destinations. The bots are
//not in order in the input file, so we'll have to figure out the list sizes in
//order to allocate memory.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>



//Chip carrier information. We'll reuse the same structure type for bots and
//outputs to keep the implementation simple.
typedef struct
{
	int selfNum;
	int chip1;
	int chip2;
	void *lowDest;
	void *highDest;
} sBot;

typedef struct
{
	int chip;
	void *dest;
} sValue;


//Helpful constants
#define LINE_LEN 64


void Give_Chip(sBot *bot, int chipNum);


int main(int argc, char **argv)
{
	char line[LINE_LEN];
	FILE *inFile;
	sBot *swarm, *outputs;
	sValue *values;
	char *next;
	int numBots, numOutputs, numValues, n, bot, nextValue;
	
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

	//Read the lines one at a time to find the total number of bots, outputs,
	//and starting values.
	numBots = 0;
	numOutputs = 0;
	numValues = 0;
	while (fgets(line, LINE_LEN, inFile) != NULL)
	{
		//Look for bots first. We don't know how many bots are in the line, so
		//keep looking until we've found them all. The delightfully-named
		//strstr() function gives us a pointer to the first occurence of a
		//substring within a string.
		next = strstr(line, "bot");
		while (next != NULL)
		{
			//Get the bot's number
			n = atoi(next + 4);
			
			//Update the max if necessary
			if (n > numBots)
				numBots = n;
			
			//Skip ahead to look for the next substring
			next = strstr(next + 4, "bot");
		}
			
		//Now look for outputs using the same methodology
		next = strstr(line, "output");
		while (next != NULL)
		{
			n = atoi(next + 7);
			if (n > numOutputs)
				numOutputs = n;
			next = strstr(next + 7, "output");
		}
		
		//Finally, search for starting values. There can only be one value on a
		//line and the chip numbers aren't sequential, so we'll keep count by
		//incrementing the counter variable.
		next = strstr(line, "value");
		if (next != NULL)
			numValues++;
	}
	
	//Bots and outputs are both numbered starting at zero, so the count of each
	//is one higher than the maximum number.
	numBots++;
	numOutputs++;
	
	//Allocate memory for the bots, outputs, and values
	swarm = malloc(numBots * sizeof(sBot));
	outputs = malloc(numOutputs * sizeof(sBot));
	values = malloc(numValues * sizeof(sValue));
	if (swarm == NULL || outputs == NULL || values == NULL)
	{
		fprintf(stderr, "Error allocating memory: %s\n\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
	//Initialize the structure arrays
	for (n = 0; n < numBots; n++)
	{
		swarm[n].selfNum = n;
		swarm[n].chip1 = -1;
		swarm[n].chip2 = -1;
		swarm[n].lowDest = NULL;
		swarm[n].highDest = NULL;
	}
	
	for (n = 0; n < numOutputs; n++)
	{
		//We'll make the output numbers negative to distinguish them from bots
		outputs[n].selfNum = -n;
		outputs[n].chip1 = -1;
		outputs[n].chip2 = -1;
		outputs[n].lowDest = NULL;
		outputs[n].highDest = NULL;
	}
	
	for (n = 0; n < numValues; n++)
	{
		values[n].chip = -1;
		values[n].dest = NULL;
	}
	
	//Now rewind the file and parse the instructions for real
	nextValue = 0;
	rewind(inFile);
	while (fgets(line, LINE_LEN, inFile) != NULL)
	{
		if (strstr(line, "value") != NULL)
		{
			//The value lines aren't numbered, so we'll have to manually keep
			//track of how many we've used.
			n = atoi(line + 6);
			next = strstr(line, "bot");
			bot = atoi(next + 4);
			values[nextValue].chip = n;
			values[nextValue].dest = &swarm[bot];
			nextValue++;
		} else
		{
			//We know the line starts with "bot", so let's save some time and
			//just get the number.
			bot = atoi(line + 4);
			
			//We don't know whether to look for a bot or an output next, so we
			//have to look for "low" and skip ahead.
			next = strstr(line, "low") + 7;
			
			//strcmp() is picky about string comparison -- if the second
			//argument is shorter than the first, it counts as less-than even
			//if the strings are otherwise identical. Using strncmp() and
			//limiting the character count gets around this. I could just check
			//the first character, but I'm trying not to over-optimize. :-)
			if (strncmp(next, "bot", 3) == 0)
			{
				//Now we can assign the low destination
				n = atoi(next + 4);
				swarm[bot].lowDest = &swarm[n];
			} else
			{
				//Outputs work the same way
				n = atoi(next + 7);
				swarm[bot].lowDest = &outputs[n];
			}
			
			//Now skip ahead to after the "high to" and do the same thing
			next = strstr(line, "high") + 8;
			if (strncmp(next, "bot", 3) == 0)
			{
				n = atoi(next + 4);
				swarm[bot].highDest = &swarm[n];
			} else
			{
				n = atoi(next + 7);
				swarm[bot].highDest = &outputs[n];
			}
		}
	}	
	
	//Close the file as soon as we're done with it
	fclose(inFile);
	
	//Now for the fancy recursive part. Each of the values represents the root
	//of an interconnected binary tree. By traversing the tree starting at each
	//root, we can exhaustively cover every branch in the tree. The easiest way
	//to do this is with recursion.
	//
	//"But wait!" I hear you say, "Doesn't recursion use a ton of stack space?
	//Isn't this dangerous?" Not really. The tree is small (~250 nodes), and
	//we only pass two parameters to our recursive function. There's some CPU
	//state that needs to be saved, but it's not a big deal here. Tree depth
	//grows logarithmically, so I'm not sure it's a big deal on bigger trees,
	//either. With a compiler that supports tail call optimization, the stack
	//penalty is eliminated altogether. I'm not sure if any C compilers do that,
	//though. We could always do it in assembly. ;-)
	for (n = 0; n < numValues; n++)
	{
		//Each value gives one chip to one bot. Only two of the values point to
		//the same bot, but don't worry, I promise this will work.
		Give_Chip((sBot *)values[n].dest, values[n].chip);
	}
	
	return EXIT_SUCCESS;
}


//Helper function for giving chips to bots. The first chip received goes in
//chip1. Once that's full, the second goes in chip2. Once chip2 is filled, the
//bot is free to give its chips to their destinations. We make this happen by
//calling the function recursively on the destinations. The calls bottom out
//when a destination receives its first chip.
void Give_Chip(sBot *bot, int chipNum)
{
	if (bot->chip1 == -1)
	{
		bot->chip1 = chipNum;
	} else if (bot->chip2 == -1)
	{
		bot->chip2 = chipNum;
		
		//While we're here, print the answer!
		if ((bot->chip1 == 61 && bot->chip2 == 17) ||
		    (bot->chip1 == 17 && bot->chip2 == 61))
			printf("Bot #%d is the answer\n", bot->selfNum);
		
		//Figure out which chip is bigger, then call this function recursively
		//on the destinations.
		if (bot->chip1 > bot->chip2)
		{
			Give_Chip((sBot *)bot->highDest, bot->chip1);
			Give_Chip((sBot *)bot->lowDest, bot->chip2);
		} else
		{
			Give_Chip((sBot *)bot->highDest, bot->chip2);
			Give_Chip((sBot *)bot->lowDest, bot->chip1);
		}
	} else
	{
		fprintf(stderr, "Error: Bot %d is full! Can't take chip %d\n",
		                                                 bot->selfNum, chipNum);
		exit(EXIT_FAILURE);
	}
}
