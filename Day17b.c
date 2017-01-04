//Day17b.c
//
//Question 2: What is the length of the longest path to the bottom-right room?
//
//We have to assume here that an infinite path is impossible -- that all paths
//eventually either reach the bottom-right room or hit a closed room. Since our
//paths can definitely be longer than 64 steps, we need to modify our MD5
//implementation to handle multiple data blocks.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>


//The path nodes and queue structures don't change
typedef struct sPathNode
{
	char stepTaken;
	int x, y, depth;
	struct sPathNode *parent;
} sPathNode;

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


//We need more memory for the MD5 input string, but everything else is the same.
//Technically we should resize the string at run time, but let's keep this part
//simple.
#define NONE_OPEN     0x0
#define UP_OPEN       (1 << 0)
#define DOWN_OPEN     (1 << 1)
#define LEFT_OPEN     (1 << 2)
#define RIGHT_OPEN    (1 << 3)
#define MD5SIZE       16
#define MAX_PATH_LEN  4096
#define MIN_X         1
#define MIN_Y         1
#define MAX_X         4
#define MAX_Y         4


void Copy_Path_Chars(char *dest, const sPathNode *leaf);
unsigned short Get_Door_States(const char *inputPath);
sPathNode *Create_Child_Node(sPathNode *parent);
void Init_Queue(sQueue *queue);
void Enqueue(sQueue *queue, sPathNode *foundNode);
sQueueNode Dequeue(sQueue *queue);
void Delete_Queue(sQueue *queue);
void MD5(const char *input, uint8_t *output);
void Print_MD5_Hash(uint_least8_t *hash);


int main(int argc, char **argv)
{
	//Now we need to save the longest path
	sQueue queue;
	sPathNode *next, *new;
	sPathNode start;
	char *inputPath;
	int inputLen, longestPath;
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
	
	//The starting node is still the same
	start.stepTaken = '\0';
	start.x = MIN_X;
	start.y = MIN_Y;
	start.depth = 0;
	start.parent = NULL;
	
	//Create a queue
	Init_Queue(&queue);

	//Now we continue building the tree until it's complete. This is indicated
	//by a queue underrun. (Dequeue() now returns null for an underrun.)
	next = &start;
	longestPath = 0;
	while (next != NULL)
	{
		//If we've reached the bottom-right, we don't need to continue the path.
		//All we have to do is check whether this is a new longest path.
		if (next->x == MAX_X && next->y == MAX_Y)
		{
			if (next->depth > longestPath)
				longestPath = next->depth;

			next = Dequeue(&queue).node;
			continue;
		}
		
		//No change to the door state or child node creation
		Copy_Path_Chars(inputPath + inputLen, next);
		doorState = Get_Door_States(inputPath);
		
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

		next = Dequeue(&queue).node;
	}
	
	//Delete the queue as soon as we're done with the search
	Delete_Queue(&queue);
	
	//Print the longest path length
	printf("The longest path is %d steps long\n", longestPath);
	
	//Free the MD5 input buffer
	free(inputPath);
	
	return EXIT_SUCCESS;
}


//The MD5 input strings still work the same way
void Copy_Path_Chars(char *dest, const sPathNode *leaf)
{
	const sPathNode *nextNode;
	
	dest[leaf->depth] = '\0';
	
	nextNode = leaf;
	while (nextNode->depth > 0)
	{
		dest[nextNode->depth - 1] = nextNode->stepTaken;
		nextNode = nextNode->parent;
	}
}

//The door states still work the same way
unsigned short Get_Door_States(const char *inputPath)
{
	uint_least8_t hash[MD5SIZE];
	unsigned short state = NONE_OPEN;
	
	MD5(inputPath, hash);

	if ((hash[0] >> 4) >= 0xb)
		state |= UP_OPEN;
	
	if ((hash[0] & 0x0f) >= 0xb)
		state |= DOWN_OPEN;
	
	if ((hash[1] >> 4) >= 0xb)
		state |= LEFT_OPEN;
	
	if ((hash[1] & 0x0f) >= 0xb)
		state |= RIGHT_OPEN;
	
//	printf("%s\n", inputPath);
//	Print_MD5_Hash(hash);
//	printf("\n%hx\n", state);
	
	return state;
}

//Child nodes are still the same
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


//We just need to modify Dequeue(). Everything else is unchanged.

//Remove and return the next path node from the queue. If the queue is empty,
//return a null node.
sQueueNode Dequeue(sQueue *queue)
{
	sQueueNode *oldest;
	sQueueNode retVal;
	
	//Return a queue node with a null pointer if empty
	if (queue->numElements == 0)
	{
		retVal.node = NULL;
		retVal.next = NULL;
		return retVal;
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


//We have the modify the MD5 code to take data of any length. This is now a full
//implementation of RFC 1321.

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

//Now we can get into the actual function
void MD5(const char *input, uint_least8_t *output)
{
	//The input must be padded to a multiple of 512 bits, so we need a buffer
	//for that. We also need four 32-bit state variables initialized to specific
	//values.
	uint32_t *msg, *buffer;
	uint32_t A = A_INIT;
	uint32_t B = B_INIT;
	uint32_t C = C_INIT;
	uint32_t D = D_INIT;
	uint32_t AA, BB, CC, DD;
	size_t msgLen, paddedLen, bufLen;
	int block, op, base;
	
	//Get the length of the input message and determine the total input length
	//after padding. The rules here are that we always have to pad, the padded
	//length must be a multiple of 512 bits, and the minimum number of padding
	//bits is 65, of which 64 are the message length. This means that any
	//message length congruent to >=448 mod 512 must add an additional 512-bit
	//block composed entirely of padding. The bitwise trick for rounding up
	//to the next multiple of 512 comes from the excellent book Hacker's Delight
	//by Henry Warren.
	msgLen = strlen(input) * CHAR_BIT;
	paddedLen = (msgLen + 511) & -512;
	if (msgLen % 512 >= 448 || msgLen % 512 == 0)
		paddedLen += 512;

	//Allocate a buffer to hold the padded input. Using calloc() clears the
	//buffer automatically, which helps with the padding.
	bufLen = paddedLen / (CHAR_BIT * sizeof(uint32_t));
	buffer = calloc(bufLen, sizeof(uint32_t));
	if (buffer == NULL)
	{
		fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	//Copy the input data into the message buffer
	strncpy((char *)buffer, input, bufLen * sizeof(uint32_t));
	
	//Pad the data to 512 bits. There are three parts to this:
	//
	//1. The bit right after the end of the message becomes a one. We can do
	//   this using strcat() for convenience.
	//2. All the bits from there to bit 447 become zeros. This happened when we
	//   called calloc() earlier.
	//3. The last 64 bits hold the message length (in bits) before the padding.
	//   The value is stored as two 32-bit words in little-endian order, so the
	//   length will be in the first word. We can use a uint64_t pointer to
	//   access both words at once.
	strcat((char *)buffer, "\x80");
	
	//Let's break this one down:
	//
	//buffer + bufLen - 2      A pointer to a 32-bit word 64 bits before the end
	//                         of the message.
	//*(uint64_t *)            Cast the above to a pointer to a 64-bit value at
	//                         the same address, then dereference (access) it.
	*(uint64_t *)(buffer + bufLen - 2) = msgLen;

	//Now that the message is padded, we can get to the core algorithm, which
	//processes one 512-bit block of data at a time. The process consists of
	//four rounds, each of which has 16 operations. The operations all follow
	//the same basic pattern. Here's round 1:
	//
	//    A = B + (A + F(B,C,D) + msg[k] + T[i])<<<s
	//
	//In later rounds, F is replaced by G, H, and I, respectively. The
	//parameters vary in each operation within a round. The state variables and
	//shift values repeat every four operations, so loops of four seem like a
	//natural choice. Only the data indices are totally irregular, so we'll
	//use a look-up table for those.
	for (block = 0; block < paddedLen/512; block++)
	{
		//Point to the start of the next data block
		msg = buffer + block*16;
		
		//Save the starting values of the state variables so we can add them
		//back in later.
		AA = A;
		BB = B;
		CC = C;
		DD = D;
		
		//Do the four rounds
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

		//Add back in the saved state variable values
		A += AA;
		B += BB;
		C += CC;
		D += DD;
	}
	
	//Free the message buffer once we're done with it
	free(buffer);
	
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

//Print an MD5 hash in the canonical little-endian hexadecimal format
void Print_MD5_Hash(uint_least8_t *hash)
{
	int c;
	
	for (c = 0; c < MD5SIZE; c++)
	{
		printf("%02x", hash[c]);
	}
}
