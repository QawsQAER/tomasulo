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

   res_add = (reservation_entry_t *) malloc(sizeof(reservation_entry_t) * ADD_RES_NUM);
   res_mul = (reservation_entry_t *) malloc(sizeof(reservation_entry_t) * MUL_RES_NUM);
   reg_file = (register_entry *) malloc(sizeof(register_entry) * REG_NUM);
   memset(res_add,0,sizeof(reservation_entry_t) * ADD_RES_NUM);
   memset(res_mul,0,sizeof(reservation_entry_t) * MUL_RES_NUM);
   memset(reg_file,0,sizeof(register_entry) * REG_NUM);
   return;
}

void writeResult(writeResult_t *theResult) {
   return;
}

/*
   In execute(), actually 'issue' in the textbook
   1. look for an instruction that can be executed, with the longest waiting time in reservation station
   2. put it on executation
   3. increase the waiting time for all the other reservation entry.
*/
int execute(mathOp mathOpType, executeRequest_t *executeRequest) {
   if(mathOpType == add)
   {

   }
   else if(mathOpType == mult)
   {

   }
   else
   {
      fprintf(stderr,"Error: Invalid mathOpType encountered in execute()\n");
      exit(0);
   }
   return (0);
}

/*
   In issue(), actually 'dispatch' in the textbook, should
   1. check whether can be issued
   2. check whether need to get the source from previous operand
   3. check whether need to update the register file's tag
*/
int issue(instruction_t *theInstruction) {
   printf("%d %d %d %d\n", theInstruction->instructionType, theInstruction->dest, theInstruction->op1, theInstruction->op2);
   switch(theInstruction->instructionType)
   {
      case addImm:

      break;
      
      case addReg:
      break;
      
      case multImm:
      break;

      case multReg:
      break;
   }
   return (1);
}

int checkDone(int registerImage[NUM_REGISTERS]) {
   int i;

   for (i=0; i < NUM_REGISTERS; i++) {
      registerImage[i] = 0;
   }
   return (1);
}

uint32_t get_next_ins_idx(reservation_entry_t * res)
{
   uint32_t idx = 0;
   uint32_t life_max = 0;
   uint32_t max_idx = 0;
   uint32_t num_of_entries = 0;
   if(res == res_add)
   {num_of_entries = ADD_RES_NUM;}
   else
   {num_of_entries = MUL_RES_NUM;}

   for(uint32_t count = 0; count < num_of_entries;count++)
   {
      if(res[count].busy && res[count].ready && (res[count].life > life_max))
      {
         life_max = count;
         max_idx = count;
      }
   }
   return max_idx;
}

void my_get_config(uint32_t * add_res_num, uint32_t * mul_res_num)
{
	FILE *fd = fopen(CONFIG_FILE,"r");
	if(fd == NULL)
	{
		fprintf(stderr,"Cannot open %s, please check the path",CONFIG_FILE);
		exit(0);
	}
	return 0;
}
