//Day25a.c
//
//The twenty-fifth and last challenge is to modify our assembly interpreter from
//Day 23 with a new instruction, which outputs data. This instruction can be
//used to generate a clock signal -- alternating zeros and ones. As a reminder,
//the previous instructions were:
//    
//    cpy x y    Copy x (a register or an integer) into register y
//    inc x      Increase the value of register x by one
//    dec x      Decrease the value of register x by one
//    jnz x y    Move y instructions in the program (positive is forward,
//               negative is backward) if x (register or integer) is not zero
//    tgl x      Toggle the instruction that is x instructions away
//
//The tgl instruction does not appear in my input, but there are now jnz
//instructions with two constants (e.g. jnz 1 -4), which is an unconditional
//branch. The new instruction is
//
//    out x      Outputs x (integer or register) as the next value
//
//There are four registers -- a, b, c, and d, all of which (except for a) are
//initialized to zero.
//
//As on Day 23 and Day 12, our input is a series of lines, each of which has one
//assembly instruction.
//
//Question 1: What is the lowest positive integer that can be used to initialize
//register a and cause the code to output a repeating clock signal (0, 1, 0, 1,
//etc.)?
//
//To solve this puzzle, we need to implement the new instruction (and update our
//instruction handling to allow two constants -- maybe we should have done that
//before?). We also need a way of verifying that the output will repeat forever.
//But how? We can't run the program forever. An automated program analyzer would
//be way too hard, and I'm guessing it would be equivalent to the Halting
//Problem in any case. No, what's needed here is some old-fashioned brainpower.
//The program is 30 lines long. Look at the final line:
//
//    jnz 1 -21
//
//An unconditional branch to somewhere near the start of the program. That sure
//looks like an outer loop to me! Now check out the first nine lines. I've
//added some comments to clarify their purposes. (The semicolon is the usual
//comment character in assembly languages.)
//
//1   cpy a d     ;Copy the initial value to d
//2   cpy 7 c     ;Initialize c to 7
//3   cpy 365 b   ;Initialize b to 365
//4   inc d       ;Add b to d (three lines)
//5   dec b
//6   jnz b -2
//7   dec c       ;Repeat the addition c times (two lines)
//8   jnz c -5
//9   cpy d a     ;Target of unconditional branch at end of program
//
//The value 365 * 7 (2555) is added to our initial value from register a. The
//result is stored in d. Every time the program repeats, 2555 is copied into
//register a. So the program is divided into two parts -- initialization and
//the main loop. The main loop doesn't use register d at all, so its value never
//changes. The other registers are initialized starting with the cpy:
//
//9   cpy d a     ;Store 2555 into a
//10  jnz 0 0     ;Does nothing -- effectively a NOP (no operation)
//11  cpy a b     ;Copy 2555 from a to b
//12  cpy 0 a     ;Reset a to zero
//13  cpy 2 c     ;Store 2 into c
//
//Thus, all we have to do is run the program until it reaches the final branch:
//
//30  jnz 1 -21   ;Restart the main loop
//
//At this point, our generated output will start to repeat. If the output is
//correct so far, it will continue to be correct forever! We could just check
//the PC after each instruction, but since this is the last day let's have some
//fun. We're going to implement one more instruction -- a breakpoint:
//
//    brk x [y]  Halt program execution. The argument(s) are ignored.
//
//We can programmatically replace the last jnz instruction with a brk. This
//instruction will toggle a state variable in the machine structure, which we'll
//check in the main instruction processing loop. This is actually similar to how
//real-life breakpoints can be implemented in microcontrollers. Instead of
//monitoring the program counter (which takes hardware support), you can set as
//many breakpoints as you want by having a debugger overwrite the instructions.
//When you delete the breakpoint or continue execution, the debugger (driven by
//PC software) puts the old instruction back.
//
//As a side note, when writing assembly you'd normally branch to labels (named
//locations in your code) instead of hard-coded offsets. The assembler would
//then convert the label names to offsets during assembly. However, if you only
//have disassembled code, the offsets are all you have to work with! A nice
//disassembler will generate numbered label names to help you out, but obviously
//that's not as helpful as the original code.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <limits.h>


//For Day 25, we'll add a boolean variable to indicate whether the machine is
//halted. We'll make the output part of the machine too, which keeps the
//instruction processing consistent. Everything else is the same as Day 23 --
//registers, PC, and (to support tgl) the program itself.
typedef struct sMachine
{
	int a, b, c, d;
	unsigned int pc;
	struct sInstruction *program;
	bool halted;
	int *output;
	size_t outputSize;
	size_t bufSize;
} sMachine;

//Instructions haven't changed since the original puzzle on Day 12
typedef void (*instPtr)(int *op1, int *op2, sMachine *machine);

typedef struct sInstruction
{
	instPtr operation;  //The typedef is a pointer, so don't add an asterisk!
	int *op1;
	int *op2;
	int constant;
} sInstruction;

//For Day 25, we're adding out and brk
void cpy(int *op1, int *op2, sMachine *machine);
void inc(int *op1, int *op2, sMachine *machine);
void dec(int *op1, int *op2, sMachine *machine);
void jnz(int *op1, int *op2, sMachine *machine);
void tgl(int *op1, int *op2, sMachine *machine);
void out(int *op1, int *op2, sMachine *machine);
void brk(int *op1, int *op2, sMachine *machine);


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
	int i, numInst, initVal;
	bool outputMatch;
	
	//The usual command line argument check and input file opening
	if (argc != 2)
	{
		fprintf(stderr, "Usage:\n\tDay25 <input filename>\n\n");
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
	
	
	//Read the file one line at a time, converting each one to an instruction.
	i = 0;
	while (fgets(line, sizeof(line), inFile) != NULL)
	{
		Parse_Instruction(line, &state, &state.program[i]);
		i++;
	}
	
	//Change the final instruction to a breakpoint
	state.program[i-1].operation = &brk;

	//Close the file as soon as we're done with it.
	fclose(inFile);
	
	//Allocate memory for the output. As usual, we'll start with a moderate size
	//and double it every time we need more.
	state.output = malloc(128 * sizeof(int));
	state.outputSize = 0;
	state.bufSize = 128;
	if (state.output == NULL)
	{
		fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
	//Instead of just executing the program, we need to try with different a
	//values to find the lowest one that gives the output we want. We also need
	//the while loop to check whether the machine is halted by a breakpoint.
	for (initVal = 0; initVal < INT_MAX; initVal++)
	{
		//Initialize the machine state
		state.a = initVal;
		state.b = 0;
		state.c = 0;
		state.d = 0;
		state.pc = 0;
		state.halted = false;
		state.outputSize = 0;
		
		printf("%5d\r", initVal);
		
		//Run the program until we hit the breakpoint
		while (state.program[state.pc].operation != NULL && !state.halted)
		{
			(*state.program[state.pc].operation)(state.program[state.pc].op1,
												 state.program[state.pc].op2,
												 &state);
		}
		
		//Check whether the output has the clock nature (0, 1, 0, 1...)
		outputMatch = true;
		for (i = 0; i < state.outputSize; i++)
		{
			//Even digits should be 0 and odd digits should be 1
			if (i % 2 == 0 && state.output[i] != 0)
				outputMatch = false;
			else if (i % 2 == 1 && state.output[i] != 1)
				outputMatch = false;
		}
		
		//There also need to be an even number of digits. Otherwise, we could
		//start and end on a zero, which would give a double zero on the repeat.
		if (state.outputSize % 2 != 0)
			outputMatch = false;
		
		if (outputMatch)
			break;
	}
	
	//Free the program memory and output buffer
	free(state.program);
	free(state.output);
	
	//Print the final state of the machine
	printf("Final state\n");
	Print_Machine_State(&state);
	
	//Print the answer
	printf("Correct initial value of a for clock generation: %d\n", initVal);
	
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

//Helper function for parsing text instructions. This is the same as on previous
//days except for added support for out and brk.
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
		case 't': inst->operation = &tgl;  break;
		case 'o': inst->operation = &out;  break;  //New in Day 25
		case 'b': inst->operation = &brk;  break;  //New in Day 25
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

//out instruction. Adds an integer to the output (resizing the buffer if
//necessary), then increments the PC.
void out(int *op1, int *op2, sMachine *machine)
{
	//Resize the output buffer if necessary
	if (machine->outputSize >= machine->bufSize)
	{
		machine->bufSize *= 2;
		machine->output = realloc(machine->output,
		                                        machine->bufSize * sizeof(int));
		if (machine->output == NULL)
		{
			fprintf(stderr, "Error reallocating memory: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	//Add the argument's value to the output and increment the PC
	machine->output[machine->outputSize] = *op1;
	machine->outputSize++;
	machine->pc++;
}

//brk instruction. Sets the machine state to halted, then advances the PC (just
//in case we ever want to resume execution).
void brk(int *op1, int *op2, sMachine *machine)
{
	machine->halted = true;
	machine->pc++;
}
