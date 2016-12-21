//Day21a.c
//
//The twenty-first challenge is to follow a series of instructions for
//scrambling a password (a string of letters).
//
//Our input is a series of lines, each of which contains one instruction for
//altering the password string. The valid instructions are:
//
//    swap position X with position Y
//    swap letter X with letter Y
//    rotate left X steps
//    rotate right X steps
//    rotate based on position of letter X
//    reverse positions X through Y
//    move position X to position Y
//
//I'll get into the details of the instructions in the code.
//
//Question 1: Given the list of instructions in the input, what is the result of
//scrambling "abcdefgh" (8 letters)?
//
//To solve this puzzle, we have to follow the instructions. But first we have to
//decide how to represent the string. Moving a letter from one position to
//another would be easier with a linked list, but most of the other operations
//would be easier with an array. Let's go with an array.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


//The longest command is rotation based on letter position, which is 37 chars
#define LINE_LEN    40


void Rotate_Right(char *str, int size);
void Rotate_Left(char *str, int size);


int main(int argc, char **argv)
{
	FILE *inFile;
	char line[LINE_LEN];
	char *password;
	char letter1, letter2, temp;
	int index1, index2, i, p, pwLen;

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
	
	//Read the instructions from the file one at a time, modifying the password
	//as we go. I bet a functional language like Haskell could do something
	//really neat with function composition here.
	while(fgets(line, LINE_LEN, inFile) != NULL)
	{
		//Decode the instruction. We have to use strncmp() to limit the length
		//of the comparison, otherwise the full line would never compare equal
		//to a single word.
		if (strncmp(line, "swap", sizeof("swap")-1) == 0)
		{
			if (strncmp(line + 5, "position", sizeof("position")-1) == 0)
			{
				//Swap the characters at two positions
				sscanf(line, "swap position %d with position %d", &index1,
				                                                       &index2);

				//Swapping is easy!
				temp = password[index1];
				password[index1] = password[index2];
				password[index2] = temp;
			} else
			{
				//Swap two letters in the whole string
				sscanf(line, "swap letter %c with letter %c", &letter1,
				                                                      &letter2);
																	  
				//Search for the characters to swap one letter at a time
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
				//Rotate the whole string based on letter position. Find the
				//index of the first matching letter. Rotate the string to the
				//right once, then again a number of times equal to the index,
				//then (if the index is at least 4) once more.
				sscanf(line, "rotate based on position of letter %c", &letter1);
				
				//The C standard library's string search functions only return
				//a pointer to the discovered character or substring. To get the
				//index, we need to subtract a pointer to the start of the
				//string from this pointer. The difference between the two
				//addresses is the index. (No scaling is needed since the size
				//of a char is 1.) In general, subtracting two pointers gets you
				//a value of type ptrdiff_t that may not fit into an int. But in
				//this case, the string is only 8 characters, so we know it's
				//okay.
				index1 = strchr(password, letter1) - password;
				if (index1 >= 4)
					index1 += 2;
				else
					index1 += 1;
				
				//Rotate the string to the right that many times
				for (p = 0; p < index1; p++)
				{
					Rotate_Right(password, pwLen);
				}
			} else
			{
				if (strncmp(line + 7, "left", sizeof("left")-1) == 0)
				{
					//Rotate the whole string left
					sscanf(line, "rotate left %d", &index1);
					
					//Repeat a single rotation as many times as are necessary
					for (p = 0; p < index1; p++)
					{
						Rotate_Left(password, pwLen);
					}
				} else
				{
					//Rotate the whole string right
					sscanf(line, "rotate right %d", &index1);
					
					//Repeat a single rotation as many times as are necessary
					for (p = 0; p < index1; p++)
					{
						Rotate_Right(password, pwLen);
					}
				}
			}
		} else if (strncmp(line, "reverse", sizeof("reverse")-1) == 0)
		{
			//Reverse the order of the letters between two indices (inclusive)
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
			//Remove the letter at one index and insert it after another index
			sscanf(line, "move position %d to position %d", &index1, &index2);
			
			//This is equivalent to rotating part of the string
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
			
	//Print the scrambled password
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
