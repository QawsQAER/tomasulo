///////////////////////////////////////////////////////////////////////
//
// tomasulo.h
// Copyright (C) 2005 Carnegie Mellon University
//
// Description:
// Tomasulo data structures
//
// All code changes go to tomasulo.c
//
//


#include <stdio.h>

#ifndef _tomasulo_
#define _tomasulo_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//--------------------------------------------------------------------------------------
// Total number of defined architectural registers
// This will remain fixed at 32 during our grading

#define NUM_REGISTERS 32

typedef enum { add = 0, mult = 1 } mathOp;
typedef enum { addImm = 0, addReg = 1, multImm = 2, multReg = 3 } instType;

typedef struct {
   int tag;
   int op1;
   int op2; 
} executeRequest_t;

typedef struct {
   int tag;
   int value;
} writeResult_t;

typedef struct {
   instType instructionType;
   int dest;
   int op1;
   int op2;
} instruction_t;


//--------------------------------------------------------------------------------------
// Function: void initTomasulo()
//   
// This is called before any traces are processed for instantiating and allocating
// your own data structures (e.g., register file, reservation stations, etc.)
//

void initTomasulo();


//--------------------------------------------------------------------------------------
// Function: int issue(instruction_t *theInstruction)
//   
// This function is called up to the number of times PER cycle as specified by
// the issue rate in your configuration file. The argument passed is an instruction_t
// data structure that contains the appropriate register operands and the instruction
// type. The instruction type is specified as an enumeration instType. 
//
// This function should determine if an instruction can be issued to a reservation
// station (if a slot is available). Return 1 in this function if a slot is available
// and return 0 otherwise. 
//
// DO NOT FREE THE INSTRUCTION POINTER!
//

int issue(instruction_t *theInstruction);



//--------------------------------------------------------------------------------------
// Function: int execute(mathOp mathOpType, executeRequest_t *executeRequest)
//   
// This function is called twice every cycle, once for each reservation station (add, multiply)
// to query if an instruction has all its operands ready for execution. 
// 
// This function should return 0 if no instructions are ready to begin ALU processing.
// If an instruction is ready to be processed, return 1 and fill in the tag, op1, and op2 fields of the
// executeRequest_t data structure pointer passed to you.  The result of this request along with
// the tag you associated with this request will be returned to you when writeResult is called (see below).
//
// DO NOT FREE THE EXECUTEREQUEST POINTER!
//
// 

int execute(mathOp mathOpType, executeRequest_t *executeRequest);


//--------------------------------------------------------------------------------------
// Function: void writeResult(writeResult_t *theResult)
//   
// This function is called when an instruction's result has completed (as a consequence 
// of you calling the function 'execute' cycles before). The writeResult data structure contains a
// tag (which you provided when you called 'execute') and the result of the computation
// you requested.
//
// DO NOT FREE THE WRITERESULT POINTER!

void writeResult(writeResult_t *theResult);


//--------------------------------------------------------------------------------------
// Function: int checkDone(int registerImage[NUM_REGISTERS])
//   
// After all of the instructions in the trace file have been read, this function is called on every cycle 
// to check if the simulator should retire. This function should return 1 when all of the instructions
// have drained out of your Tomasulo implementation.  When you return 1, copy all of the your internal
// register values into the registerImage provided so that we can print out the final values for checking.
// 

int checkDone(int registerImage[NUM_REGISTERS]);

#endif
