//Day17a.c
//
//The seventeenth challenge is to navigate a 4x4 grid of rooms ("the grid").
//The doors in each room open and close based on the path taken through the
//grid. It is possible to become trapped in a room if the wrong path is taken.
//
//The state of the doors in the current room is determined by the MD5 hash of a
//string of characters. The string consists of a passcode (our input) followed
//by zero or more letters representing our movements within the grid -- 'U' for
//up, 'D' for down, 'L' for left, and 'R' for right. The first hex digit of the
//resulting hash gives the state of the upper door -- 0-a means closed, b-f
//means open. The second digit gives the state of the lower door, the third
//gives the state of the left door, and the fourth gives the state of the right
//door.
//
//Question 1: Starting from the top-left room, what is the shortest path to
//reach the bottom-right room?
//
//As we've seen on Day 13, a breadth-first search is a good way to find a
//shortest path. However, this maze cannot be represented by a simple unchanging
//graph. The state of the maze changes after every move, so the shortest path
//may involve a loop! The need to build up the MD5 string character by character
//suggests that a tree structure might be better, so let's give that a try. Each
//node in the tree will represent one step along a path. There can be up to four
//exits from a room, so each node can have as many as four child nodes. By
//constructing the tree in a breadth-first fashion, we're guaranteed that the
//first time we reach the exit will be along a shortest path!


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>


//Each node in the tree will consist of a letter (the direction taken to get
//there), the coordinates of the room, and a link to the parent node. To make
//building the MD5 input and reporting the answer easier, we'll also store the
//depth in the tree.
typedef struct sPathNode
{
	char stepTaken;
	int x, y, depth;
	struct sPathNode *parent;
} sPathNode;

//As on Day 13, we'll be using a linked-list queue to store newly-discovered
//routes.
typedef struct sQueueNode
{
	sPathNode *node;
	struct sQueueNode *next;
} sQueueNode;

typedef struct
{
	unsigned long numElements;
	sQueueNode *first, *last;
} sQueue;


//Door states and other constants. Instead of doing shifting here, we could use
//0x1, 0x2, 0x4, and 0x8 for the state bits. Experienced programmers can handle
//that sort of thing in their sleep, but sometimes it's easier to spell things
//out, particularly when you have 16+ bits to define. It's just a matter of
//style; there's no functional difference.
#define NONE_OPEN     0x0
#define UP_OPEN       (1 << 0)
#define DOWN_OPEN     (1 << 1)
#define LEFT_OPEN     (1 << 2)
#define RIGHT_OPEN    (1 << 3)
#define MD5SIZE       16
#define MAX_PATH_LEN  128
#define MIN_X         1
#define MIN_Y         1
#define MAX_X         4
#define MAX_Y         4


void MD5(const char *input, uint8_t *output);
void Copy_Path_Chars(char *dest, const sPathNode *leaf);
unsigned short Get_Door_States(const char *inputPath);
sPathNode *Create_Child_Node(sPathNode *parent);
void Init_Queue(sQueue *queue);
void Enqueue(sQueue *queue, sPathNode *foundNode);
sQueueNode Dequeue(sQueue *queue);
void Delete_Queue(sQueue *queue);


int main(int argc, char **argv)
{
	sQueue queue;
	sPathNode *next, *new;
	sPathNode start;
	char *inputPath;
	int inputLen;
	unsigned short doorState;

	//The usual command line argument processing
	if (argc != 2)
	{
		fprintf(stderr, "Usage:\n\tDay17 <input data>\n");
		return EXIT_FAILURE;
	}
	inputLen = strlen(argv[1]);
	
	//Allocate memory for the MD5 input string and copy the input into it
	inputPath = malloc(inputLen + MAX_PATH_LEN + 1);
	if (inputPath == NULL)
	{
		fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	strcpy(inputPath, argv[1]);
	
	//Initialize the node for the starting location. This is the head of the
	//exploration tree.
	start.stepTaken = '\0';
	start.x = MIN_X;
	start.y = MIN_Y;
	start.depth = 0;
	start.parent = NULL;
	
	//Create a queue
	Init_Queue(&queue);

	//Start building the tree. Stop when we see the target coordinates. As on
	//Day 13, we're relying on the fact that a breadth-first search discovers
	//nodes along a shortest path.
	next = &start;
	while (next->x != MAX_X || next->y != MAX_Y)
	{
		//Construct the MD5 input string and get the current door states
		Copy_Path_Chars(inputPath + inputLen, next);
		doorState = Get_Door_States(inputPath);
		
		//Create a child node for each possible step. We have to be sure not to
		//go off the edge of the grid here! Note that the logical && here allows
		//for a safe combination of & and >.
		if ((doorState & UP_OPEN) && next->y > MIN_Y)
		{
			new = Create_Child_Node(next);
			new->stepTaken = 'U';
			new->y--;
			Enqueue(&queue, new);
		}
		if ((doorState & DOWN_OPEN) && next->y < MAX_Y)
		{
			new = Create_Child_Node(next);
			new->stepTaken = 'D';
			new->y++;
			Enqueue(&queue, new);
		}
		if ((doorState & LEFT_OPEN) && next->x > MIN_X)
		{
			new = Create_Child_Node(next);
			new->stepTaken = 'L';
			new->x--;
			Enqueue(&queue, new);
		}
		if ((doorState & RIGHT_OPEN) && next->x < MAX_X)
		{
			new = Create_Child_Node(next);
			new->stepTaken = 'R';
			new->x++;
			Enqueue(&queue, new);
		}

		//Cheap syntax trick! Dequeue() returns an sQueueNode, but we don't need
		//to store it in a variable to access it -- we can use function return
		//values directly. (We could've just made Dequeue() return the node
		//pointer directly, but I wanted to show this off. C's syntax is more
		//flexible than it appears at first glance.)
		next = Dequeue(&queue).node;
	}
	
	//Delete the queue as soon as we're done with the search
	Delete_Queue(&queue);
	
	//Print the shortest path
	Copy_Path_Chars(inputPath, next);
	printf("Shortest path: %s\n", inputPath);
	
	//Free the MD5 input buffer
	free(inputPath);
	
	return EXIT_SUCCESS;
}


//Helper function for building the MD5 input string. To do this, we need to
//climb the tree, copying the path character at each node in reverse order.
void Copy_Path_Chars(char *dest, const sPathNode *leaf)
{
	const sPathNode *nextNode;
	
	//Write the null character first
	dest[leaf->depth] = '\0';
	
	//Copy characters until we reach the root
	nextNode = leaf;
	while (nextNode->depth > 0)
	{
		dest[nextNode->depth - 1] = nextNode->stepTaken;
		nextNode = nextNode->parent;
	}
}

//Helper function for determining which doors in a room are open based on the
//path taken. The return value is a bitmask which gives the four door states.
//This is much more efficient (and about as easy to deal with) compared to using
//a struct of four booleans.
unsigned short Get_Door_States(const char *inputPath)
{
	uint8_t hash[MD5SIZE];
	unsigned short state = NONE_OPEN;
	
	//Calculate the MD5sum of the input string and path
	MD5(inputPath, hash);

	//The first character in the MD5 output string is the upper hex digit of the
	//first byte. The second is the lower hex digit of the first byte. The third
	//and fourth are the upper and lower hex digits of the second byte,
	//respectively. These are 8-bit values, so a simple right shift isolates the
	//upper four bits. Note that if you're going to do shifting you really ought
	//to use unsigned values. In particular, the result of a right shift on a
	//signed negative value is implementation-defined.
	if ((hash[0] >> 4) >= 0xb)
		state |= UP_OPEN;
	
	if ((hash[0] & 0x0f) >= 0xb)
		state |= DOWN_OPEN;
	
	if ((hash[1] >> 4) >= 0xb)
		state |= LEFT_OPEN;
	
	if ((hash[1] & 0x0f) >= 0xb)
		state |= RIGHT_OPEN;
	
	return state;
}

//Helper function for creating a new child node. This hides the error checking
//for malloc() and does some helpful initialization.
sPathNode *Create_Child_Node(sPathNode *parent)
{
	sPathNode *new;
	
	new = malloc(sizeof(sPathNode));
	if (new == NULL)
	{
		fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	new->x = parent->x;
	new->y = parent->y;
	new->depth = parent->depth + 1;
	new->parent = parent;
	
	return new;
}


//This is the queue code from Day 13, with types changed but otherwise unaltered

//Initialize the step queue
void Init_Queue(sQueue *queue)
{
	queue->numElements = 0;
	queue->first = NULL;
	queue->last = NULL;
}

//Add a step to the queue
void Enqueue(sQueue *queue, sPathNode *foundNode)
{
	sQueueNode *new;
	
	//Create the new step node for the linked list
	new = malloc(sizeof(sQueueNode));
	if (new == NULL)
	{
		fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	new->node = foundNode;
	new->next = NULL;
	
	//Add the node to the tail of the list. If the list is empty, we need to
	//update the queue pointers to cover the new list.
	if (queue->numElements > 0)
		queue->last->next = new;
	else
		queue->first = new;
	queue->last = new;
	queue->numElements++;
}

//Remove and return the next path node from the queue
sQueueNode Dequeue(sQueue *queue)
{
	sQueueNode *oldest;
	sQueueNode retVal;
	
	//In an infinite maze, we should never run out of nodes, so trying to get a
	//node from an empty list is a showstopping error.
	if (queue->numElements == 0)
	{
		fprintf(stderr, "Error: Queue underrun!\n");
		exit(EXIT_FAILURE);
	}
	
	//Copy the oldest node
	oldest = queue->first;
	retVal = *oldest;

	//Delete the oldest node from the linked list. We can only update the queue
	//pointers if there are still other nodes left in the list.
	if (queue->numElements > 1)
		queue->first = oldest->next;
	free(oldest);
	queue->numElements--;
	
	return retVal;
}

//Free all the memory used by the queue
void Delete_Queue(sQueue *queue)
{
	sQueueNode *next;
	
	while (queue->numElements > 0)
	{
		next = queue->first->next;
		free(queue->first);
		queue->first = next;
		queue->numElements--;
	}
}


//This is the MD5 code from Day 5. Nothing has changed since then.

//MD5 is fairly complicated, so I'm going to simplify just a bit by limiting the
//input to 64 characters (512 bits), which is the size of a single data block.
//The Wikipedia explanation of the algorithm is a bit too concise, so I
//recommend reading the primary source (RFC 1321) instead. All of the weird
//functions and constants here come directly from that document. Most of the
//values in the algorithm have bit sizes specified, so we'll be using integer
//types from stdint.h, which was introduced in C99.

//We need to do left rotates on 32-bit values. Strangely, C doesn't have an
//operator for this even though it's a very common CPU instruction.
#define lrot(val, bits) (((val) << (bits)) | ((val) >> (32 - (bits))))

//We also need a constant table. These values are based on sines, and are
//provided in the RFC.
const uint32_t T[64] = {0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
                        0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
						0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
						0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
						0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
						0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
						0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
						0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
						0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
						0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
						0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
						0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
						0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
						0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
						0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
                        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};
						
//This table holds the order in which we access the data array. There's no
//pattern that I can see.
const int k[64] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,  //R1
                   1, 6, 11, 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12,  //R2
				   5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2,  //R3
                   0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2, 9}; //R4

//We also need to define some helper functions. We don't want to add function
//call overhead, so let's use the preprocessor. ("But... the preprocessor is
//evil!" I hear you say. "The optimizer will take care of everything!" Maybe on
//x86 and x64 it will, but good luck getting a magical level of optimization on
//some obscure CPU architecture. Anyway, always remember to put parentheses
//around macro definitions and every input variable. This keeps the order of
//operations correct even if you pass in something like a + 5.
#define F(x, y, z) (((x) & (y)) | (~(x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & ~(z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | ~(z)))
#define ROUND1(a, b, c, d, x, s, i) (a) = (b)+lrot((a) + F(b,c,d) + (x) + (i), (s))
#define ROUND2(a, b, c, d, x, s, i) (a) = (b)+lrot((a) + G(b,c,d) + (x) + (i), (s))
#define ROUND3(a, b, c, d, x, s, i) (a) = (b)+lrot((a) + H(b,c,d) + (x) + (i), (s))
#define ROUND4(a, b, c, d, x, s, i) (a) = (b)+lrot((a) + I(b,c,d) + (x) + (i), (s))

//Finally, some miscellaneous constants
#define A_INIT       0x67452301
#define B_INIT       0xefcdab89
#define C_INIT       0x98badcfe
#define D_INIT       0x10325476
#define MSG_WORDLEN  16

//Now we can get into the actual function
void MD5(const char *input, uint8_t *output)
{
	//The input must be padded to 512 bits, so we need a buffer for that. We
	//also need four 32-bit state variables initialized to specific values.
	uint32_t msg[MSG_WORDLEN];
	uint32_t A = A_INIT;
	uint32_t B = B_INIT;
	uint32_t C = C_INIT;
	uint32_t D = D_INIT;
	int msgLen, op, base;
	
	//Clear the message buffer and copy the input into it. Also get the length
	//of the input in bits -- we'll need it later.
	memset(msg, 0x00000000, MSG_WORDLEN * sizeof(uint32_t));
	msgLen = strlen(input) * CHAR_BIT;
	
	//One of the neat (and dangerous) things about C is that you can access the
	//binary representation of a variable however you want via pointers and
	//typecasting. For example, you can access the binary representation of a
	//floating point value as an integer. Here, we're doing something more
	//pedestrian. We need to access the input data as 32-bit words, but we
	//received it as a char string. Instead of doing a bunch of shifting and
	//ORing to combine bytes into 32-bit words, we can just copy the string
	//directly into the message buffer using strcpy() and a pointer cast. We
	//dont have to worry about the null character, because it's just a zero!
	strcpy((char *)msg, input);
	
	//Pad the data to 512 bits. There are three parts to this:
	//
	//1. The bit right after the end of the message becomes a one. We can do
	//   this using strcat() for convenience.
	//2. All the bits from there to bit 447 become zeros. This happened when we
	//   cleared the buffer earlier.
	//3. The last 64 bits hold the message length (in bits) before the padding.
	//   The value is stored as two 32-bit words in little-endian order (so the
	//   length will be in the first word. We can use a uint64_t pointer to
	//   access both words at once.
	strcat((char *)msg, "\x80");
	
	//Let's break this one down:
	//
	//msg + MSG_WORDLEN - 2    A pointer to a 32-bit word 64 bits before the end
	//                         of the message
	//*(uint64_t *)            Cast the above to a pointer to a 64-bit value at
	//                         the same address, then dereference (access) it
	*(uint64_t *)(msg + MSG_WORDLEN - 2) = msgLen;

	//Now that the message is padded, we can get to the core algorithm. This
	//consists of four rounds, each of which has 16 operations. The operations
	//all follow the same basic pattern. Here's round 1:
	//
	//    a = b + (a + F(b,c,d) + msg[k] + T[i])<<<s
	//
	//In later rounds, F is replaced by G, H, and I, respectively. The
	//parameters vary in each operation within a round. The state variables and
	//shift values repeat every four operations, so loops of four seem like a
	//natural choice. Only the data indices are totally irregular, so we'll
	//use a look-up table for those.
	for (op = 0; op < 16; op += 4)
	{
		base = 0 + op;
		ROUND1(A, B, C, D, msg[k[base+0]], 7, T[base+0]);
		ROUND1(D, A, B, C, msg[k[base+1]], 12, T[base+1]);
		ROUND1(C, D, A, B, msg[k[base+2]], 17, T[base+2]);
		ROUND1(B, C, D, A, msg[k[base+3]], 22, T[base+3]);
	}
	for (op = 0; op < 16; op += 4)
	{
		base = 16 + op;
		ROUND2(A, B, C, D, msg[k[base+0]], 5, T[base+0]);
		ROUND2(D, A, B, C, msg[k[base+1]], 9, T[base+1]);
		ROUND2(C, D, A, B, msg[k[base+2]], 14, T[base+2]);
		ROUND2(B, C, D, A, msg[k[base+3]], 20, T[base+3]);
	}
	for (op = 0; op < 16; op += 4)
	{
		base = 32 + op;
		ROUND3(A, B, C, D, msg[k[base+0]], 4, T[base+0]);
		ROUND3(D, A, B, C, msg[k[base+1]], 11, T[base+1]);
		ROUND3(C, D, A, B, msg[k[base+2]], 16, T[base+2]);
		ROUND3(B, C, D, A, msg[k[base+3]], 23, T[base+3]);
	}
	for (op = 0; op < 16; op += 4)
	{
		base = 48 + op;
		ROUND4(A, B, C, D, msg[k[base+0]], 6, T[base+0]);
		ROUND4(D, A, B, C, msg[k[base+1]], 10, T[base+1]);
		ROUND4(C, D, A, B, msg[k[base+2]], 15, T[base+2]);
		ROUND4(B, C, D, A, msg[k[base+3]], 21, T[base+3]);
	}
		
	//For the full algorithm, we're supposed to save the values of A, B, C, and
	//D before the operations, then add them back in afterward. Because we only
	//ever process one data block, we can just add the initial values instead.
	A += A_INIT;
	B += B_INIT;
	C += C_INIT;
	D += D_INIT;
	
	//The four state variables are the output. Unfortunately, the standard hex
	//representation prints the bytes in little-endian order, which is the
	//opposite of how 32-bit values are printed! To make the result unambiguous,
	//we're going to return the 128-bit result as an 8-bit array. We'll do this
	//using one last pointer conversion trick. This time, we'll use the typecast
	//pointer with an array index.
	((uint32_t *)output)[0] = A;
	((uint32_t *)output)[1] = B;
	((uint32_t *)output)[2] = C;
	((uint32_t *)output)[3] = D;
}
