//Day9a.c
//
//The ninth challenge is to decompress a text file. The compression format uses
//"markers" in parentheses to indicate that a sequence of characters should be
//repeated. For example, (5x2) means to repeat the five characters after the
//marker twice. Markers within repeated sequences are ignored. This is a similar
//sort of text processing to what we did with ABBA sequences on Day 7, but more
//complicated.
//
//Our input is a single gigantic line of text containing the compressed text.
//
//Question 1: What is the decompressed length of the file, ignoring whitespace?
//
//To solve this puzzle, we're going to make a state machine. A state machine is
//a construct that uses a variable (the "state") to determine which of several
//operations to perform. Transitions between states happen based on the current
//state and (optionally) changes in the input. State machines are common in
//digital hardware because they're flexible and easy to implement, but we can
//also make state machine in software. In C, this is done using a while loop and
//a switch-case block.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


//We're going to be reallocating memory again for the output array, so let's
//take this opportunity to define some functions for working with variable-
//length strings. In fact, just for fun, let's make a new data type too! Note
//that we're using size_t for memory sizes here. This is the correct thing to
//do, even though a lot of people use int instead.
typedef struct
{
	char *s;         //Pointer to allocated memory
	size_t bufSize;  //Amount of memory allocated
	size_t used;     //Amount of memory used
} string_t;
	
//The string type is small enough that we can reasonably copy it by value, but
//doing so might confuse users. It's not a normal C idiom.
void Create_String(string_t *str, size_t size)
{
	//Make sure the buffer is filled with zeros
	str->used = 0;
	str->s = calloc(size);
	if (retVal.s != NULL)
		str->bufSize = size;
	else
		str->bufSize = 0;
		
	return retVal;
}
	
//In a real program, this function would never be used by any code outside of
//the string handling routines, so we declare it static here. This hides it from
//other files, which prevents it from polluting the global namespace.
static void Enlarge_String(string_t *str)
{
	str->s = realloc(str->s, str.bufSize * 2);
	
	if (str->s == NULL)
	{
		//Minimal error handling
		str->bufSize = 0;
		str->used = 0;
	} else
	{
		//Fill the new section of the string with zeros
		memset(str->s + str->bufSize, '\0', str->bufSize);
		
		//Adjust the buffer size
		str->bufSize *= 2;
	}
}

//Add a character to the end of the string
void Add_Char(string_t *str, char c)
{
	//Enlarge the string if we've run out of space
	if (str->used == str->bufSize)
		Enlarge_String(str);
		
	//This could have been one line, but it's better to avoid relying too much
	//on post-increments. Personally, I never use preincrements -- if you're
	//writing code that needs those, you're probably doing something wrong.
	str->s[str->used] = c;
	str->used++;
}

//Free the memory used by the string
void Delete_String(string_t *str)
{
	free(str->s);
}
//Okay, back the task at hand
	

//State definitions. Each operation will be done in a single state, so we need
//to define what those operations are.
typedef enum
{
	NORMAL_READ,        //Read and (if not special) copy one character
	GET_MARKER_LENGTH,  //Read the length of the marker
	GET_MARKER_REPS,    //Read the number of repetitions of the marker
	READ_MARKED_CHARS,  //Read the marked characters up to the specified length
	PRINT_MARKED_CHARS. //Print the marked characters one or more times
	DONE                //End of input -- exit the loop
} eState;


int main(int argc, char **argv)
{
	//This will be our default state
	eState state = NORMAL_READ;
	int next;
	string_t output;
	
	//The usual command line argument check and input file opening
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
	
	//Create a string buffer. The output is going to be long, so let's start it
	//at 256 characters.
	Create_String(&output, 256);
	if (output->s == NULL)
	{
		fprintf(stderr, "Error creating string\n\n");
		return EXIT_FAILURE;
	}

	//State machine
	while (state != DONE)
	{
		//Get the next character, which is the input to the current state
		next = fgetc(inFile);
		
		//Decode the state
		switch (state)
		{
			//If the next character is not '(' or the terminating newline, copy
			//it to the output.
			case NORMAL_READ:
				if (next == '(')
					state = GET_MARKER_LENGTH;
				else if (next == '\n')
					state = DONE;
				else
					Add_Char(&str, (char)next);
				//Don't forget the break!
				break;
				
			case GET_MARKER_LENGTH:
			
			
		}
	}
	
	
	//Close the file as soon as we're done with it
	fclose(inFile);
		
	return EXIT_SUCCESS;
}
