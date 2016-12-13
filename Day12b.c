//Day12b.c
//
//Question 2: If register c is initialized to 1, what is the value in register a
//after the program executes?
//
//To solve this puzzle, we need to change one line of code. Woohoo!


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

//The assembly interpreter is untouched
typedef struct
{
	int a, b, c, d;
	unsigned int pc;
} sMachine;

void cpy(int *op1, int *op2, unsigned int *pc);
void inc(int *op1, int *op2, unsigned int *pc);
void dec(int *op1, int *op2, unsigned int *pc);
void jnz(int *op1, int *op2, unsigned int *pc);

typedef void (*instPtr)(int *op1, int *op2, unsigned int *pc);

typedef struct
{
	instPtr operation;  //The typedef is a pointer, so don't add an asterisk!
	int *op1;
	int *op2;
	int constant;
} sInstruction;

//Helper function
void Parse_Instruction(char *text, sMachine *state, sInstruction *inst);


int main(int argc, char **argv)
{
	//No changes here...
	FILE *inFile;
	char line[32];
	sInstruction program[32];
	sMachine state;
	int i;
	
	//...or here.
	for (i = 0; i < 32; i++)
	{
		program[i].operation = NULL;
		program[i].op1 = NULL;
		program[i].op2 = NULL;
		program[i].constant = 0;
		line[i] = '\0';
	}
	
	//This is the only difference. Everything from here on is the same.
	state.a = 0;
	state.b = 0;
	state.c = 1;
	state.d = 0;
	state.pc = 0;

	//The usual command line argument check and input file opening
	if (argc != 2)
	{
		fprintf(stderr, "Usage:\n\tDay12 <input filename>\n\n");
		return EXIT_FAILURE;
	}

	inFile = fopen(argv[1], "r");
	if (inFile == NULL)
	{
		fprintf(stderr, "Error opening file: %s\n\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
	//Read the file one line at a time, converting each one to an instruction.
	i = 0;
	while (fgets(line, sizeof(line), inFile) != NULL)
	{
		Parse_Instruction(line, &state, &program[i]);
		i++;
	}

	//Close the file as soon as we're done with it.
	fclose(inFile);
	
	//Execute the program. Terminate when a null instruction is encountered.
	while (program[state.pc].operation != NULL)
	{
		(*program[state.pc].operation)(program[state.pc].op1,
		                               program[state.pc].op2,
									   &state.pc);
	}
	
	//Print the final state of the machine
	printf("Final state\n");
	printf("       a        b        c        d       PC\n");
	printf("--------------------------------------------\n");
	printf("%8d %8d %8d %8d %8d\n", state.a, state.b, state.c, state.d,
	                                                                  state.pc);
		
	return EXIT_SUCCESS;
}


//Helper function for parsing text instructions. This function takes a null-
//terminated string and modifies it via strtok(). The other parameters are the
//machine state (to get the register pointers) and a pointer to an sInstruction.
void Parse_Instruction(char *text, sMachine *state, sInstruction *inst)
{
	char *token;
	
	//Look ma, it's functional programming! :-)  The first character tell us
	//the instruction.
	token = strtok(text, " ");
	switch (token[0])
	{
		case 'c': inst->operation = &cpy;  break;
		case 'i': inst->operation = &inc;  break;
		case 'd': inst->operation = &dec;  break;
		case 'j': inst->operation = &jnz;  break;
		default:
			fprintf(stderr, "Unknown instruction %s\n\n", token);
			exit(EXIT_FAILURE);
	}
	
	//The next token contains either a letter (register) or a number (constant)
	token = strtok(NULL, " ");
	switch (token[0])
	{
		case 'a': inst->op1 = &state->a;    break;
		case 'b': inst->op1 = &state->b;    break;
		case 'c': inst->op1 = &state->c;    break;
		case 'd': inst->op1 = &state->d;    break;
		default:
			//It's a constant! Store the number in the structure variable and
			//make the operand pointer point to it.
			inst->constant = atoi(token);
			inst->op1 = &inst->constant;
			break;
	}
	
	//We'll only have a second operand if the instruction is cpy or jnz
	if (inst->operation == &cpy || inst->operation == &jnz)
	{
		//This is basically the same as the first operand
		token = strtok(NULL, " ");
		switch (token[0])
		{
			case 'a': inst->op2 = &state->a;    break;
			case 'b': inst->op2 = &state->b;    break;
			case 'c': inst->op2 = &state->c;    break;
			case 'd': inst->op2 = &state->d;    break;
			default:
				inst->constant = atoi(token);
				inst->op2 = &inst->constant;
				break;
		}
	}
}
		
//cpy instruction. Copies operand 1's value into operand 2, then increments
//the PC.
void cpy(int *op1, int *op2, unsigned int *pc)
{
	*op2 = *op1;
	
	//*pc++ would increment the pointer instead
	(*pc)++;
}

//inc instruction. Increments operand 1, then increments the PC.
void inc(int *op1, int *op2, unsigned int *pc)
{
	(*op1)++;
	(*pc)++;
}

//dec instruction. Decrements operand 1, then increments the PC.
void dec(int *op1, int *op2, unsigned int *pc)
{
	(*op1)--;
	(*pc)++;
}

//jnz instruction. Checks whether operand 1 is nonzero. If so, adds operand 2's
//value to the PC. If not, increments the PC.
void jnz(int *op1, int *op2, unsigned int *pc)
{
	if (*op1 != 0)
		*pc += *op2;
	else
		(*pc)++;
}
	
	
	