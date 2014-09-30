///////////////////////////////////////////////////////////////////////
//
// tomasulo.c
// Copyright (C) 2005 Carnegie Mellon University
//
// Description:
// Your code goes in this file. Please read tomasulo.h carefully.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tomasulo.h"

void initTomasulo() {
   return;
}

void writeResult(writeResult_t *theResult) {
   return;
}

int execute(mathOp mathOpType, executeRequest_t *executeRequest) {
   return (0);
}

int issue(instruction_t *theInstruction) {
   printf("%d %d %d %d\n", theInstruction->instructionType, theInstruction->dest, theInstruction->op1, theInstruction->op2);
   return (1);
}

int checkDone(int registerImage[NUM_REGISTERS]) {
   int i;

   for (i=0; i < NUM_REGISTERS; i++) {
      registerImage[i] = 0;
   }
   return (1);
}
