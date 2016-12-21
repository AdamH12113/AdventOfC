//Day21b.c
//
//Question 2: We now need to unscramble an existing password by reversing the
//scrambling process. What is the unscrambled version is "fbgdceah"?
//
//Well, this got ugly fast. Now we have to modify all of our instruction
//processing to do the reverse of what's specified in the file, *and* we have to
//follow the instructions in reverse. I don't see a nice way of doing this, so
//let's slog on through. Here's what we're up against:
//
//* The swap and reverse commands are symmetrical -- no change needed.
//* The rotate left/right commands need to reverse their direction.
//* The move command needs its indices swapped.
//* The rotate based on position command... well, let's cross that bridge when
//  we come to it. :-(


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


//Now we need to know the number of instructions
#define LINE_LEN           40
#define NUM_INSTRUCTIONS  100


void Rotate_Right(char *str, int size);
void Rotate_Left(char *str, int size);


int main(int argc, char **argv)
{
	//Instead of reading the instructions one at a time, we're going to read
	//them all at once. This is going to be ugly enough already, so let's just
	//declare a fixed-size buffer.
	FILE *inFile;
	char instructions[NUM_INSTRUCTIONS][LINE_LEN];
	char *password, *line;
	char letter1, letter2, temp;
	int index1, index2, i, p, pwLen, inst;

	//The usual command line argument check and input file opening
	if (argc != 3)
	{
		fprintf(stderr, "Usage:\n\tDay21 <input filename> <password>\n\n");
		return EXIT_FAILURE;
	}

	inFile = fopen(argv[1], "r");
	if (inFile == NULL)
	{
		fprintf(stderr, "Error opening file: %s\n\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
	//None of our operations change the length of the password, so we can modify
	//the copy in argv[2]. This is explicitly allowed by the C standard.
	password = argv[2];
	pwLen = strlen(password);
	
	//Read all of the instructions from the file, one line at a time
	inst = 0;
	while (fgets(instructions[inst], LINE_LEN, inFile) != NULL)
	{
		inst++;
	}
	
	//Process the instructions one at a time in reverse
	for (inst = NUM_INSTRUCTIONS - 1; inst >= 0; inst--)
	{
		//To minimize code modification, we can turn line into a pointer to the
		//current instruction.
		line = instructions[inst];
		
		//The instruction decode is the same
		if (strncmp(line, "swap", sizeof("swap")-1) == 0)
		{
			if (strncmp(line + 5, "position", sizeof("position")-1) == 0)
			{
				//Swap the characters at two positions -- still works!
				sscanf(line, "swap position %d with position %d", &index1,
				                                                       &index2);

				//Swapping is still easy!
				temp = password[index1];
				password[index1] = password[index2];
				password[index2] = temp;
			} else
			{
				//Swap two letters in the whole string -- still works!
				sscanf(line, "swap letter %c with letter %c", &letter1,
				                                                      &letter2);
																	  
				//No change to the letter swapping code
				for (i = 0; i < pwLen; i++)
				{
					if (password[i] == letter1)
						password[i] = letter2;
					else if (password[i] == letter2)
						password[i] = letter1;
				}
			}
		} else if (strncmp(line, "rotate", sizeof("rotate")-1) == 0)
		{
			if (strncmp(line + 7, "based", sizeof("based")-1) == 0)
			{
				//Rotate the whole string based on letter position. This... this
				//is going to be obnoxious. The exact instructions from the
				//Advent of Code page for this are:
				//
				//1. Find the index of letter X *before* any rotation.
				//2. Rotate right once.
				//3. Rotate right a number of times equal to the index.
				//4. If the index is at least 4, rotate right once more.
				//
				//I don't think this is reversible in general, so we'll have to
				//rely on the fact that our password has no repeated letters.				
				sscanf(line, "rotate based on position of letter %c", &letter1);
				
				//First, let's find the current index
				index1 = strchr(password, letter1) - password;
				
				//The current index was reached by one of two formulas:
				//
				//    new_index = (2*old_index + 1) % pwLen, if old_index < 4
				//    new_index = (2*old_index + 2) % pwLen, if old_index >= 4
				//
				//This conveniently guarantees that each possible old_index maps
				//to a unique new_index! Now all we have to do is make a look-up
				//table:
				//
				//    old_index  new_index
				//    0          1
				//    1          3
				//    2          5
				//    3          7
				//    4          2
				//    5          4
				//    6          6
				//    7          0
				//
				//This depends on the password length being 8, but other lengths
				//don't seem to generate unique values...
				const int newToOld[8] = {7, 0, 4, 1, 5, 2, 6, 3};
				index2 = newToOld[index1];
				if (index2 >= 4)
					index2 += 2;
				else
					index2 += 1;
				
				//All that's left is to rotate the string back to the left
				for (p = 0; p < index2; p++)
				{
					Rotate_Left(password, pwLen);
				}
			} else
			{
				if (strncmp(line + 7, "left", sizeof("left")-1) == 0)
				{
					//Rotate the whole string left -- reverse this
					sscanf(line, "rotate left %d", &index1);
					
					//Repeat a single rotation as many times as are necessary
					for (p = 0; p < index1; p++)
					{
						//Now it's right!
						Rotate_Right(password, pwLen);
					}
				} else
				{
					//Rotate the whole string right -- reverse this
					sscanf(line, "rotate right %d", &index1);
					
					//Repeat a single rotation as many times as are necessary
					for (p = 0; p < index1; p++)
					{
						//Now it's left!
						Rotate_Left(password, pwLen);
					}
				}
			}
		} else if (strncmp(line, "reverse", sizeof("reverse")-1) == 0)
		{
			//Reverse the order of the letters between two indices (inclusive).
			//This doesn't need to change!
			sscanf(line, "reverse positions %d through %d", &index1, &index2);
			p = index2 - index1;
			
			//Swap the letters. We only need to go halfway through the string.
			for (i = 0; i <= p/2; i++)
			{
				temp = password[index1 + i];
				password[index1 + i] = password[index2 - i];
				password[index2 - i] = temp;
			}
		} else if (strncmp(line, "move", sizeof("move")-1) == 0)
		{
			//Remove the letter at one index and insert it after another index.
			//This needs its indices reversed!
			sscanf(line, "move position %d to position %d", &index1, &index2);
			
			//Reverse the indices
			p = index1;
			index1 = index2;
			index2 = p;
			
			//No change to the actual rotation code
			if (index1 > index2)
			{
				//Rotate right to wrap the character around to the left
				Rotate_Right(password + index2, index1 - index2 + 1);
			} else
			{
				//Rotate left to wrap the character around to the right
				Rotate_Left(password + index1, index2 - index1 + 1);
			}
		} else
		{
			fprintf(stderr, "Invalid instruction: %s\n", line);
			fclose(inFile);
			return EXIT_FAILURE;
		}
	}
	
	//Close the file as soon as we're done with it
	fclose(inFile);
			
	//Print the unscrambled password
	printf("Scrambled password: %s\n", password);
	
	return EXIT_SUCCESS;
}


//Helper function for rotating the string to the right
void Rotate_Right(char *str, int size)
{
	int c;
	char temp;
	
	//Save the character at the right end
	temp = str[size-1];
	
	//Shift the remaining characters
	for (c = size-1; c > 0; c--)
	{
		str[c] = str[c-1];
	}
	
	//Put the saved character at the left end
	str[0] = temp;
}
	
//Helper function for rotating the string to the left
void Rotate_Left(char *str, int size)
{
	int c;
	char temp;
	
	//Save the character at the left end
	temp = str[0];
	
	//Shift the remaining characters
	for (c = 0; c < size-1; c++)
	{
		str[c] = str[c+1];
	}
	
	//Put the saved character at the right end
	str[size-1] = temp;
}
