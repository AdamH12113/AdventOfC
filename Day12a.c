//Day12a.c
//
//The twelfth challenge is to determine a password that results from executing a
//sequence of assembly instructions. The instructions are in a made-up language
//with only four instructions:
//
//    cpy x y    Copy x (a register or an integer) into register y
//    inc x      Increase the value of register x by one
//    dec x      Decrease the value of register x by one
//    jnz x y    Move y instructions in the program (positive is forward,
//               negative is backward) if x (register or integer) is not zero
//
//There are four registers -- a, b, c, and d. They are all initialized to zero.
//
//These instructions are fairly typical of actual assembly languages. The cpy
//instructions is normally called something like "mov", and the destination
//register normally comes first. (You'll notice that memcpy() uses the same
//order. That's probably not a coincidence.) But the key element is the jnz
//instruction. This type of instruction is called a conditional branch, and it's
//how CPUs do flow control. Higher-level constructs like if-else statements and
//while loops exist to automate and foolproof conditional branching.
//
//It may also interest you to know that the combination of inc, dec, and jnz is
//Turing complete! For example, you can do addition like this:
//
//    Assembly    C Equivalent
//    -----------------------------
//                do {
//    inc a           a = a + 1
//    dec b           b = b - 1
//    jnz b -2    } while (b != 0);   <-- Equivalent to a = a + b
//
//For more information, see these links:
//https://en.wikipedia.org/wiki/Turing_machine_equivalents#Register_machine_models
//http://softwareengineering.stackexchange.com/questions/132385/what-makes-a-language-turing-complete
//
//Question 1: After executing the code in our program input, what is the value
//in register a?
//
//To solve this puzzle, we need to implement an interpreter for the language.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

//The state of our simulated machine consists of the registers (a, b, c, and d)
//along with the PC (program counter). The PC tells which instruction we're on.
//Normally we increment it after each instruction, but for a jnz instruction we
//have to add the offset in the instruction.
typedef struct
{
	int a, b, c, d;
	unsigned int pc;
} sMachine;

//We'll implement the instructions as functions. For reasons that will shortly
//become clear, all of them will have the same arguments. The jnz instruction
//modifies the PC, so we'll make that a parameter as well.
void cpy(int *op1, int *op2, unsigned int *pc);
void inc(int *op1, int *op2, unsigned int *pc);
void dec(int *op1, int *op2, unsigned int *pc);
void jnz(int *op1, int *op2, unsigned int *pc);

//Instructions consists of an operation (represented here by a function pointer)
//and one or two operands, represented here by integer pointers. Some
//instructions have a constant, so we'll store that here as well. A function
//pointer is like a normal pointer (it's still a memory address), except that
//you dereference it by calling the function it points to. The syntax for
//function pointers gets a bit confusing, so the easiest way to define them is
//with a typedef. We could have typedef'd the function definition itself, but
//that's a little weird.
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
	//The instructions and program are small, so we'll create fixed-size buffers
	//for both.
	FILE *inFile;
	char line[32];
	sInstruction program[32];
	sMachine state;
	int i;
	
	//Fill the line and program buffers with null values
	for (i = 0; i < 32; i++)
	{
		program[i].operation = NULL;
		program[i].op1 = NULL;
		program[i].op2 = NULL;
		program[i].constant = 0;
		line[i] = '\0';
	}
	
	//Initialize the machine state as instructed
	state.a = 0;
	state.b = 0;
	state.c = 0;
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
		//You don't have to explicitly dereference a function pointer, and you
		//don't need to use the address-of operator (&) before the function name
		//when you assign one, but it's good practice to do so anyway to remind
		//readers that there is a pointer involved.
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
	
	
	