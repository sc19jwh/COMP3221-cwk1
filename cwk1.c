//
// Starting code for coursework 1. Requires "cwk1_extra.h", which should be placed in the same directory.
//
// To compile, either use the provided makefile, or from a shell (on the school machines) type:
//
// gcc -fopenmp -Wall -o cwk1 cwk1.c
//
// where '-fopenmp' is redundant until OpenMP functionality has been added, and '-Wall' (to turn on
// all warnings) is optional but recommended.
//
// If you want to change the compiler used by the makefile, it is recommended to use the following:
//
// make CC=<alternative compiler>
//
// If you instead change the makefile directly, be careful to change it back before submitting
// (or simply do not submit your makefile), otherwise it may not run on the test machine.
//


//
// Includes
//

// Standard includes.
#include <stdio.h>
#include <stdlib.h>

// Declarations and functions specific to this coursework. This file will
// be replaced with a different version as part of the assessment, and so there is
// no point making alterations to it - it is best to only modify this file ('cwk1.c').
#include "cwk1_extra.h"
// You can read 'cwk1_extra.h' to see what it provides, but in summary:
//
// Global variables:
// int *stack;					// Stack array; allocated in allocateStack() and freed in finaliseStack().
// int maxStackSize0;			// The maximum stack size (=size of the array as allocated).
// int stackSize;				// The current stack size; initially zero.
//
// Functions:
// void allocateStack()			// Allocates memory for the stack array.
// void finaliseStack()			// Frees up memory used by the stack array.
// void displayStack( int )		// Prints the current stack. The argument is for messaging (0 for initial stack, 1 for final stack).
// int parseCommandLine( int argc, char** argv, int* initSize, int* numPop, int* invert, int* rotate )
//								// Parses the 5 command line arguments and sets the values of initSize, numPop, invert, and rotate.
//								// Prints an error message and returns -1 for invalid values.
//


//
// Functions for stack management.
//

// Push an item to the stack. Prints an error message if the stack limit has already been reached.
void pushToStack( int newItem )
{
    // sc19jwh - Code remains to ensure that it is not assumed that initStackSize ≤ maxStackSize
	if( stackSize==maxStackSize )
	{
		printf( "Cannot add to stack; already at its maximum size (of %i).\n", maxStackSize );
		return;
	}

	// sc19jwh - Critical region used given data dependencies, enclosed using #pragma omp critical
	// sc19jwh - used instead of atomic as there is more than one update operation
	#pragma omp critical
	{
		// Add the 'item' (i.e. the integer value) to the top of the stack, and also increment the stack size.
		stack[stackSize++] = newItem;
	}
}

// Removes an item from the stack but does not return the value.
void popFromStack()
{
    // sc19jwh - atomic used given it is one single update operation and thus more efficient in this context
	// sc19jwh - decrement stackSize to remove top element from stack (last element - LIFO)
	#pragma omp atomic
	stackSize--;
}

// Inverts the stack in-place; that is, the order of all elements is reversed.
void invertStack()
{
	int i, temp;
	// sc19jwh - Parallel for loop through half of the stack
	// sc19jwh - Private temp variable used so that each thread has its own copy of the variable
    #pragma omp parallel for private (temp)
    for (i = 0; i < stackSize / 2; i++) {
		// sc19jwh - Capture current element in temp variable
        temp = stack[i];
		// sc19jwh - Set i'th from bottom value to i'th from top value and vice versa
        stack[i] = stack[stackSize - 1 - i];
        stack[stackSize - 1 - i] = temp;
    }
}

// Rotates the stack down to the given value.
void rotateStack( int depth )
{
	int i, temp = stack[stackSize-depth];
	// sc19jwh - initialize a temp stack to avoid race conditions
	int* tempStack = (int*) malloc( sizeof(int) * stackSize );
	// sc19jwh - copy contents of stack into tempstack
	#pragma omp parallel for
	for( i=0; i<stackSize; i++ )
		tempStack[i] = stack[i];

    // sc19jwh - loop made parallel using omp parallel for
	#pragma omp parallel for
	for( i=0; i<depth-1; i++ )
		stack[stackSize-depth+i] = tempStack[stackSize-depth+i+1];

	stack[stackSize-1] = temp;

	// sc19jwh - free temp stack memory
	free( tempStack );
}

//
// Main
//
int main( int argc, char** argv )
{
	int i;

	//
	// Initialisation. Parse command line arguments and allocate memory for the stack.
	//

	// These are the parameters set by command line arguments. This does not include the maximum stack size,
	// which is the global variable 'maxStackSize' (defined in 'cwk1_extra.h') and is set in parseCommandLine() below.
	int initStackSize, numToPop, invertYesNo, rotateDepth;

	// Get the values from the command line. parseCommandLine() gives and error message and returns -1 for bad values.
	if( parseCommandLine(argc,argv,&initStackSize,&numToPop,&invertYesNo,&rotateDepth)==-1 )	
		return EXIT_FAILURE;

	// Allocates memory for the stack. Must also call 'finaliseStack' before termination.
	allocateStack();

	//
	// 1. Add multiple items to the stack in a loop. This loop needs to be parallelised as part of the coursework.
	//
    // sc19jwh - loop made parallel using omp parallel for
    #pragma omp parallel for
	for( i=1; i<=initStackSize; i++ )
	{
		pushToStack( i * i );
	}

	// Display the initial stack. The argument '0' means this is the initial stack.
	displayStack( 0 );		// DO NOT ALTER THIS LINE - your code must call displayStack(0) immediately after the stack is populated.

	//
	// 2. Remove multiple items from the stack. This loop needs to be parallelised as part of the coursework.
	//
    // sc19jwh - loop made parallel using omp parallel for
    #pragma omp parallel for
	for( i=1; i<=numToPop; i++ )
	{
		popFromStack();
	}

	//
	// 3. Invert the stack.
	//
	if( invertYesNo==1 ) invertStack();

	//
	// 4. Rotate the stack.
	//
	if( rotateDepth>0 ) rotateStack( rotateDepth );

	//
	// Clean up and quit.
	//

	// Display the stack in its initial state. The argument '1' means this is the final stack
	displayStack( 1 );		// DO NOT ALTER THIS LINE - your code must call displayStack(1) just before finaliseStack().

	// You MUST call finaliseStack() at this point.
	finaliseStack();

	return EXIT_SUCCESS;
}


