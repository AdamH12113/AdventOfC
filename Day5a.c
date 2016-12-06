//Day5a.c
//
//The fifth challenge is to compute a password by calculating MD5 checksums on
//varying inputs and extracting information from the sums that start with five
//zeros.
//
//Our input is a constant string of 8 characters. To get the full input
//sequence, we concatenate numbers to the end of the string, starting with 0.
//So if the input is "abcdefgh", the sequence is "abcdefgh0", "abcdefgh1",
//"abcdefgh2" ... "abcdefgh10", "abcdefgh11", etc.
//
//Question 1: Each digit of the password come from the sixth hexadecimal digit
//of a sum that starts with five zeros. If the password is the first eight such
//digits in the order that they appear, what is the password?
//
//To solve this puzzle, we have to do a bit of string concatenation and compute
//MD5 checksums, then do a simple pattern match. The normal approach to this
//is to download an existing library or source code. And indeed, if you Google
//"MD5 in C" you will find exactly that:
//
//http://openwall.info/wiki/_media/people/solar/software/public-domain-source-code/md5.h
//http://openwall.info/wiki/_media/people/solar/software/public-domain-source-code/md5.c
//
//Usage is simple -- create an MD5_CTX structure using the definition in the
//header file, pass it to MD5_Init(), call MD5_Update() with the input string,
//then call MD5_Final() with a buffer pointer to get the checksum. Treat the
//buffer as a string of 16 two-digit hex values for display purposes. The code
//is public domain, so anyone can use it.
//
//But that wouldn't be very educational. Let's make our own implementation!
//(If you'd rather not, stick to the main() function.)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

#define BUFSIZE  32
#define MD5SIZE  16
#define PWSIZE   8

void MD5(char *input, uint8_t *output);

int main(int argc, char **argv)
{
	//Arbitrary buffer sizes should always make you a little uncomfortable...
	char input[BUFSIZE];
	char numStr[BUFSIZE];
	char password[PWSIZE+1];
	uint8_t md5sum[MD5SIZE];
	int num = 0, digitsFound = 0;
	
	memset(password, '\0', sizeof(password));
	
	//This time we're taking in an actual text string instead of a file
	if (argc != 2)
	{
		fprintf(stderr, "Usage:\n\tDay5 <input string>\n\n");
		return EXIT_FAILURE;
	}

	//Loop until we're done finding digits
	while (digitsFound < PWSIZE)
	{
		//Now things are pretty simple. We need to combine the input string from
		//the command line with a number string. To get a number string, we just
		//call sprintf().
		memset(input, '\0', sizeof(input));
		strncpy(input, argv[1], BUFSIZE-1);
		sprintf(numStr, "%d", num);
		strcat(input, numStr);
		
		//Once we have our input string, we can find the MD5sum
		MD5(input, md5sum);
		
		//Each element of the MD5sum array is two hex digits (8 bits). So to
		//check whether the first five digits are zero, we need to check two
		//characters plus the top four bits of the third character.
		if (md5sum[0] == 0 && md5sum[1] == 0 && md5sum[2] < 0x10)
		{
			//Add the sixth digit of the MD5sum to the password. It's the lower
			//four bits of md5sum[2]. We already know the upper four bits are
			//zero, so we're guaranteed that it's one hex digit.
			sprintf(password + digitsFound, "%x", md5sum[2]);
			digitsFound++;
		}
		
		//Go to the next number
		num++;
	}
	
	//Print the password
	printf("%s\n", password);
	
	return EXIT_SUCCESS;
}


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
void MD5(char *input, uint8_t *output)
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
	//directly into the message buffer using strcpy() and a pointer case. We
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
