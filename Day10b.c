//Day10b.c
//
//Question 2: What do you get if you multiply together the values of the chips
//in each of outputs 0, 1, and 2?
//
//I told you tracking the outputs was a good idea! We already have everything
//we need for this answer; all we have to do is print the result.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>


//No changes here
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
	
	//No changes to any of the file operations
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

	numBots = 0;
	numOutputs = 0;
	numValues = 0;
	while (fgets(line, LINE_LEN, inFile) != NULL)
	{
		next = strstr(line, "bot");
		while (next != NULL)
		{
			n = atoi(next + 4);
			if (n > numBots)
				numBots = n;
			next = strstr(next + 4, "bot");
		}
			
		next = strstr(line, "output");
		while (next != NULL)
		{
			n = atoi(next + 7);
			if (n > numOutputs)
				numOutputs = n;
			next = strstr(next + 7, "output");
		}
		
		next = strstr(line, "value");
		if (next != NULL)
			numValues++;
	}
	
	numBots++;
	numOutputs++;
	
	swarm = malloc(numBots * sizeof(sBot));
	outputs = malloc(numOutputs * sizeof(sBot));
	values = malloc(numValues * sizeof(sValue));
	if (swarm == NULL || outputs == NULL || values == NULL)
	{
		fprintf(stderr, "Error allocating memory: %s\n\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
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
	
	nextValue = 0;
	rewind(inFile);
	while (fgets(line, LINE_LEN, inFile) != NULL)
	{
		if (strstr(line, "value") != NULL)
		{
			n = atoi(line + 6);
			next = strstr(line, "bot");
			bot = atoi(next + 4);
			values[nextValue].chip = n;
			values[nextValue].dest = &swarm[bot];
			nextValue++;
		} else
		{
			bot = atoi(line + 4);
			next = strstr(line, "low") + 7;

			if (strncmp(next, "bot", 3) == 0)
			{
				n = atoi(next + 4);
				swarm[bot].lowDest = &swarm[n];
			} else
			{
				n = atoi(next + 7);
				swarm[bot].lowDest = &outputs[n];
			}
			
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
	
	fclose(inFile);
	
	for (n = 0; n < numValues; n++)
	{
		Give_Chip((sBot *)values[n].dest, values[n].chip);
	}
	
	//All we need is one line of code
	printf("The product is %d\n", outputs[0].chip1 * outputs[1].chip1 *
	                              outputs[2].chip1);
	
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
