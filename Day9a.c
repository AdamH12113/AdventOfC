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
//a switch-case block. None of this will be super-efficient -- it's more to
//illustrate the concepts.


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
	str->s = calloc(size, sizeof(char));
	if (str->s != NULL)
		str->bufSize = size;
	else
		str->bufSize = 0;
}
	
//In a real program, this function would never be used by any code outside of
//the string handling routines, so we declare it static here. This hides it from
//other files, which prevents it from polluting the global namespace.
static void Enlarge_String(string_t *str)
{
	str->s = realloc(str->s, str->bufSize * 2);
	
	if (str->s == NULL)
	{
		//Minimal error handling -- just exit
		fprintf(stderr, "\nError reallocating string!\n\n");
		exit(EXIT_FAILURE);
	} else
	{
		//Fill the new section of the string with zeros. We just doubled the
		//size of the string, so our starting offset is half that, which is
		//equal to the old buffer size. The length is also half that.
		memset(str->s + str->bufSize, '\0', str->bufSize);
		
		//Adjust the buffer size
		str->bufSize *= 2;
	}
}

//Add a character to the end of the string
void Add_Char_To_String(string_t *str, char c)
{
	//Enlarge the string if we've run out of space. We need to make sure there's
	//enough space for the new character and a null terminator.
	if (str->used + 1 + 1 == str->bufSize)
		Enlarge_String(str);
		
	//This could have been one line, but it's better to avoid relying too much
	//on post-increments. Personally, I never use preincrements -- if you're
	//writing code that needs those, you're probably doing something wrong.
	str->s[str->used] = c;
	str->used++;
}

//Concatenate two strings
void Concatenate_Strings(string_t *dest, const string_t *src)
{
	//Allocate more memory as needed. We still need to account for the null
	//terminator.
	while (dest->used + 1 + src->used > dest->bufSize)
	{
		Enlarge_String(dest);
	}
	
	//We might as well use strcat() to do the copying
	strcat(dest->s, src->s);
	dest->used += src->used;
}
	

//Clear the string data without deallocating memory
void Reset_String(string_t *str)
{
	memset(str->s, '\0', str->used);
	str->used = 0;
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
	GET_MARKER_REPEATS, //Read the number of repetitions of the marker
	READ_MARKED_CHARS,  //Read the marked characters up to the specified length
	PRINT_MARKED_CHARS, //Print the marked characters one or more times
	DONE                //End of input -- exit the loop
} eState;


int main(int argc, char **argv)
{
	//This will be our default state
	eState state = NORMAL_READ;
	FILE *inFile;
	int mark = 0, repeats = 0;
	int next;
	string_t output, marked;
	
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
	
	//Create the output buffer. The output is going to be long, so let's start
	//it at 4096 characters.
	Create_String(&output, 4096);
	if (output.s == NULL)
	{
		fprintf(stderr, "Error creating output string\n\n");
		return EXIT_FAILURE;
	}
	
	//Create the marked character buffer. Eyeballing the input file, it seems
	//like 256 is a reasonable starting point.
	Create_String(&marked, 256);
	if (marked.s == NULL)
	{
		fprintf(stderr, "Error creating marked character string\n\n");
		Delete_String(&output);
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
				{
					//Reset the marker variables whenever we reach a new marker
					mark = 0;
					repeats = 0;
					Reset_String(&marked);
					state = GET_MARKER_LENGTH;
				} else if (next == '\n')
				{
					state = DONE;
				} else
				{
					Add_Char_To_String(&output, (char)next);
				}
				//Don't forget the break!
				break;
			
			//Get the number of characters to mark. Move on to the next state
			//when we encounter an 'x'.
			case GET_MARKER_LENGTH:
				if (next == 'x')
					state = GET_MARKER_REPEATS;
				else
					mark = 10*mark + (next - '0');
				break;
				
			//Get the number of times to repeat the marked characters. Move on
			//to the next state when we encounter a ')'.
			case GET_MARKER_REPEATS:
				if (next == ')')
					state = READ_MARKED_CHARS;
				else
					repeats = 10*repeats + (next - '0');
				break;
			
			//Read marked characters until we reach the amount specified
			case READ_MARKED_CHARS:
				Add_Char_To_String(&marked, (char)next);
				mark--;
				if (mark == 0)
					state = PRINT_MARKED_CHARS;
				break;
				
			//Copy the marked characters to the output buffer. Repeat the
			//specified number of times. This is the only state where we don't
			//process the current character, so we need to put it back before
			//returning to the normal read state.
			case PRINT_MARKED_CHARS:
				while (repeats--)
				{
					Concatenate_Strings(&output, &marked);
				}
				ungetc(next, inFile);
				state = NORMAL_READ;
				break;
			
			//It's always a good idea to have a default case
			default:
				fprintf(stderr, "Unknown state encountered\n\n");
				Delete_String(&output);
				Delete_String(&marked);
				return EXIT_FAILURE;
		}
	}
	
	
	//Close the file and delete the marked string as soon as we're done with
	//them.
	Delete_String(&marked);
	fclose(inFile);
	
	//Print the number of characters in the output string. Note the special
	//format specifier for size_t.
	printf("Number of output characters: %zu\n", output.used);
		
	return EXIT_SUCCESS;
}
