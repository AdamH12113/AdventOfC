//Day23a.c
//
//The twenty-third challenge is to modify our assembly interpreter from Day 12
//with a new instruction, which will allow us to compute a password. As a
//reminder, the old instructions were:
//
//    cpy x y    Copy x (a register or an integer) into register y
//    inc x      Increase the value of register x by one
//    dec x      Decrease the value of register x by one
//    jnz x y    Move y instructions in the program (positive is forward,
//               negative is backward) if x (register or integer) is not zero
//
//There are four registers -- a, b, c, and d. Register a is initialized to 7,
//and the others are initialized to zero. The new new instruction is:
//
//    tgl x      Toggle the instruction that is x instructions away
//
//The argument is the same kind of offset used in the jnz instruction. "Toggle"
//here means to change one instruction to another according to these rules:
//
//* tgl and dec become inc, and inc becomes dec
//* cpy becomes jnz, and jnz becomes cpy
//* If a tgl instruction toggles itself, the resulting inc is not executed yet
//* The instruction's operands are unchanged
//* If toggling produces an invalid instruction (like cpy 2 3), skip it
//
//The last rule doesn't seem to be relevant, though.
//
//Question 1: After executing the instructions in the input, what is the value
//in register a (the password)?
//
//To solve this puzzle, we have to add the tgl instruction to our old code. The
//structure we used for instructions lets us change the instructions without
//affecting the operands, but now the program itself needs to be part of the
//machine state.
//
//Even low-level programmers usually avoid self-modifying code. In a lot of
//architectures it's hard to do -- the code is in ROM, or there's protection for
//executable memory regions, or the code is only accesible through the CPU's
//program bus. Also, there's rarely a good reason to use it unless you want to
//deliberately obfuscate your code.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


//The state of our simulated machine consists of the registers (a, b, c, and d)
//along with the PC (program counter). The PC tells which instruction we're on.
//Normally we increment it after each instruction, but for a jnz instruction we
//have to add the offset in the instruction. For Day 23, we need the program
//to be part of the machine state, so we'll add a pointer here. To avoid having
//a circular dependency between the definitions of sMachine and sInstruction,
//we need to use the full struct syntax here.
typedef struct sMachine
{
	int a, b, c, d;
	unsigned int pc;
	struct sInstruction *program;
} sMachine;

//Instructions consists of an operation (represented here by a function pointer)
//and one or two operands, represented here by integer pointers. Some
//instructions have a constant, so we'll store that here as well. A function
//pointer is like a normal pointer (it's still a memory address), except that
//you dereference it by calling the function it points to. The syntax for
//function pointers gets a bit confusing, so the easiest way to define them is
//with a typedef. We could have typedef'd the function definition itself, but
//that's a little weird.
typedef void (*instPtr)(int *op1, int *op2, sMachine *machine);

typedef struct sInstruction
{
	instPtr operation;  //The typedef is a pointer, so don't add an asterisk!
	int *op1;
	int *op2;
	int constant;
} sInstruction;

//We'll implement the instructions as functions. For reasons that will shortly
//become clear, all of them will have the same arguments. Previously we passed
//the PC as a parameter, but because of the new Day 23 tgl instruction, we'll
//pass the whole machine state instead.
void cpy(int *op1, int *op2, sMachine *machine);
void inc(int *op1, int *op2, sMachine *machine);
void dec(int *op1, int *op2, sMachine *machine);
void jnz(int *op1, int *op2, sMachine *machine);
void tgl(int *op1, int *op2, sMachine *machine);


//Helper functions
void Print_Machine_State(sMachine *machine);
void Parse_Instruction(char *text, sMachine *state, sInstruction *inst);


int main(int argc, char **argv)
{
	//The instructions are still small, so we can keep using a fixed-size buffer
	//for those.
	FILE *inFile;
	char line[32];
	sMachine state;
	int i, numInst;
	
	//The usual command line argument check and input file opening
	if (argc != 2)
	{
		fprintf(stderr, "Usage:\n\tDay23 <input filename>\n\n");
		return EXIT_FAILURE;
	}

	inFile = fopen(argv[1], "r");
	if (inFile == NULL)
	{
		fprintf(stderr, "Error opening file: %s\n\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
	//Get the number of instructions in the file by counting the number of lines
	numInst = 0;
	while (fgets(line, sizeof(line), inFile) != NULL)
	{
		numInst++;
	}
	rewind(inFile);
	
	//Allocate memory for the program, with one extra location for a null
	//terminator.
	state.program = malloc((numInst+1) * sizeof(sInstruction));
	if (state.program == NULL)
	{
		fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
	//Fill the program buffer with null values
	for (i = 0; i < numInst+1; i++)
	{
		state.program[i].operation = NULL;
		state.program[i].op1 = NULL;
		state.program[i].op2 = NULL;
		state.program[i].constant = 0;
	}
	
	//Initialize the machine state as instructed
	state.a = 7;
	state.b = 0;
	state.c = 0;
	state.d = 0;
	state.pc = 0;
	
	//Read the file one line at a time, converting each one to an instruction.
	i = 0;
	while (fgets(line, sizeof(line), inFile) != NULL)
	{
		Parse_Instruction(line, &state, &state.program[i]);
		i++;
	}

	//Close the file as soon as we're done with it.
	fclose(inFile);
	
	//Execute the program. Terminate when a null instruction is encountered.
	while (state.program[state.pc].operation != NULL)
	{
		//You don't have to explicitly dereference a function pointer, and you
		//don't need to use the address-of operator (&) before the function name
		//when you assign one, but it's good practice to do so anyway to remind
		//readers that there is a pointer involved.
		(*state.program[state.pc].operation)(state.program[state.pc].op1,
		                                     state.program[state.pc].op2,
									         &state);
	}
	
	//Free the program memory
	free(state.program);
	
	//Print the final state of the machine
	printf("Final state\n");
	Print_Machine_State(&state);
	
	return EXIT_SUCCESS;
}


//Helper function for printing the machine state
void Print_Machine_State(sMachine *machine)
{
	printf("       a        b        c        d       PC\n");
	printf("--------------------------------------------\n");
	printf("%8d %8d %8d %8d %8d\n", machine->a, machine->b, machine->c,
	                                machine->d, machine->pc);
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
		case 't': inst->operation = &tgl;  break;  //New in Day 23
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
void cpy(int *op1, int *op2, sMachine *machine)
{
	*op2 = *op1;
	
	//*pc++ would increment the pointer instead
	machine->pc++;
}

//inc instruction. Increments operand 1, then increments the PC.
void inc(int *op1, int *op2, sMachine *machine)
{
	(*op1)++;
	machine->pc++;
}

//dec instruction. Decrements operand 1, then increments the PC.
void dec(int *op1, int *op2, sMachine *machine)
{
	(*op1)--;
	machine->pc++;
}

//jnz instruction. Checks whether operand 1 is nonzero. If so, adds operand 2's
//value to the PC. If not, increments the PC.
void jnz(int *op1, int *op2, sMachine *machine)
{
	if (*op1 != 0)
		machine->pc += *op2;
	else
		machine->pc++;
}

//tgl instruction. New in Day 23. Adds operand 1's value to the PC to get the
//offset of an instruction to modify, then modifies it according to the special
//rules given above, then increments the PC.
void tgl(int *op1, int *op2, sMachine *machine)
{
	sInstruction *target;

	//Get the target instruction
	target = &machine->program[machine->pc + *op1];
	
	//tgl and dec become inc, inc becomes dec, cpy becomes jnz, jnz becomes cpy
	if (target->operation == &tgl || target->operation == &dec)
		target->operation = &inc;
	else if (target->operation == &inc)
		target->operation = &dec;
	else if (target->operation == &cpy)
		target->operation = &jnz;
	else if (target->operation == &jnz)
		target->operation = &cpy;
	
	//Increment the PC
	machine->pc++;
}
