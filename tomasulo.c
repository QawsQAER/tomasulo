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

   my_get_config(&ADD_RES_NUM,&MUL_RES_NUM);
   res_add = (reservation_entry_t *) malloc(sizeof(reservation_entry_t) * ADD_RES_NUM);
   res_mul = (reservation_entry_t *) malloc(sizeof(reservation_entry_t) * MUL_RES_NUM);
   reg_file = (register_entry *) malloc(sizeof(register_entry) * NUM_REGISTERS);
   memset(res_add,0,sizeof(reservation_entry_t) * ADD_RES_NUM);
   memset(res_mul,0,sizeof(reservation_entry_t) * MUL_RES_NUM);
   memset(reg_file,0,sizeof(register_entry) * NUM_REGISTERS);
   return;
}

void writeResult(writeResult_t *theResult) {
   printf("insturction with tag %d is finished\n",theResult->tag);
   update_res(res_add,ADD_RES_NUM,theResult);
   update_res(res_mul,MUL_RES_NUM,theResult);
   uint32_t idx = 0;
   for(idx = 0;idx < NUM_REGISTERS;idx++)
   {
      if(reg_file[idx].tag == theResult->tag)
      {
         reg_file[idx].tag = 0;
         reg_file[idx].data = theResult->value;
      }
   }
   return;
}

/*
   In execute(), actually 'issue' in the textbook
   1. look for an instruction that can be executed, with the longest waiting time in reservation station
   2. put it on executation
   3. increase the waiting time for all the other reservation entry.
*/
int execute(mathOp mathOpType, executeRequest_t *executeRequest) {
   int32_t idx = 0;
   if(mathOpType == add)
   {
      idx = get_next_ins_idx(res_add);
	if(idx < 0)
         return 0;
      executeRequest->tag = idx + 1;
      executeRequest->op1 = res_add[idx].ins.op1;
      executeRequest->op2 = res_add[idx].ins.op2;
	printf("execute %d in res_add, propagating tag %d\n",idx,executeRequest->tag);
   }
   else if(mathOpType == mult)
   {
      idx = get_next_ins_idx(res_mul);
      if(idx < 0)
         return 0;
      executeRequest->tag = idx + ADD_RES_NUM + 1;
      executeRequest->op1 = res_mul[idx].ins.op1;
      executeRequest->op2 = res_mul[idx].ins.op2;
	printf("execute %d in res_mul, propagating tag %d\n",idx,executeRequest->tag);
   }
   else
   {
      fprintf(stderr,"Error: Invalid mathOpType encountered in execute()\n");
      exit(0);
   }
   return 1;
}

/*
   In issue(), actually 'dispatch' in the textbook, should
   1. check whether can be issued
   2. check whether need to get the source from previous operand
   3. check whether need to update the register file's tag
*/
int issue(instruction_t *theInstruction) {
   printf("%d %d %d %d\n", theInstruction->instructionType, theInstruction->dest, theInstruction->op1, theInstruction->op2);
   int32_t idx = 0;
   switch(theInstruction->instructionType)
   {
      case addImm:
      {
         if((idx = get_available_slot(res_add)) >= 0) 
         {
            //issuing theInstruction to res_add[idx]
            res_add[idx].busy = 1;
            res_add[idx].ins.instructionType = theInstruction->instructionType;

            res_add[idx].src1_tag = reg_file[theInstruction->op1].tag;
            res_add[idx].src2_tag = 0;
            if(res_add[idx].src1_tag == 0)
            {
               //case that the this instruction should be waiting for other instruction's output
               res_add[idx].ins.op1 = reg_file[theInstruction->op1].data;
               res_add[idx].ready = 1;
            }
            else
            {
               //get the data immediately, and the instruction is now ready to be executed
               res_add[idx].ins.op1 = 0;
               res_add[idx].ready = 0;
            }
            res_add[idx].ins.op2 = theInstruction->op2;
            res_add[idx].life = 0;

            //update the destination register.
            reg_file[theInstruction->dest].tag = idx + 1;
            return 1;
         }
         else
            return 0;
      }
      break;
      
      case addReg:
      {
         if((idx = get_available_slot(res_add)) >= 0)
         {
            //issuing theInstruction to res_add[idx]
            
            res_add[idx].busy = 1;
            res_add[idx].ins.instructionType = theInstruction->instructionType;

            res_add[idx].src1_tag = reg_file[theInstruction->op1].tag;
            res_add[idx].src2_tag = reg_file[theInstruction->op2].tag;

            if((res_add[idx].src1_tag == 0) && (res_add[idx].src2_tag == 0))
            {
               res_add[idx].ins.op1 = reg_file[theInstruction->op1].data;
               res_add[idx].ins.op2 = reg_file[theInstruction->op2].data;
               res_add[idx].ready = 1;
            }else
            {
               if(res_add[idx].src1_tag != 0)
               {res_add[idx].ins.op1 = 0;}
               else
               {res_add[idx].ins.op1 = reg_file[theInstruction->op1].data;}

               if(res_add[idx].src2_tag != 0)
               {res_add[idx].ins.op2 = 0;}
               else
               {res_add[idx].ins.op2 = reg_file[theInstruction->op2].data;}

               res_add[idx].ready = 0;
            }
            res_add[idx].life = 0;
            //update the destination register.
            reg_file[theInstruction->dest].tag = idx + 1;
            return 1;
         }
         else
            return 0;
      }
      break;
      
      case multImm:
      {
         if((idx = get_available_slot(res_mul)) >= 0)
         {
            //issuing theInstruction to res_add[idx]
            res_mul[idx].busy = 1;
            res_mul[idx].ins.instructionType = theInstruction->instructionType;

            res_mul[idx].src1_tag = reg_file[theInstruction->op1].tag;
            res_mul[idx].src2_tag = 0;
            if(res_mul[idx].src1_tag == 0)
            {
               //case that the this instruction should be waiting for other instruction's output
               res_mul[idx].ins.op1 = reg_file[theInstruction->op1].data;
               res_mul[idx].ready = 1;
            }
            else
            {
               //get the data immediately, and the instruction is now ready to be executed
               res_mul[idx].ins.op1 = 0;
               res_mul[idx].ready = 0;
            }
            res_mul[idx].ins.op2 = theInstruction->op2;
            res_mul[idx].life = 0;

            //update the destination register.
            reg_file[theInstruction->dest].tag = idx + ADD_RES_NUM +1;
            return 1;
         }
         else
            return 0;
      }
      break;

      case multReg:
      {
         if((idx = get_available_slot(res_mul)) >= 0)
         {
            res_mul[idx].busy = 1;
            res_mul[idx].ins.instructionType = theInstruction->instructionType;

            res_mul[idx].src1_tag = reg_file[theInstruction->op1].tag;
            res_mul[idx].src2_tag = reg_file[theInstruction->op2].tag;

            if((res_mul[idx].src1_tag == 0) && (res_mul[idx].src2_tag == 0))
            {
               res_mul[idx].ins.op1 = reg_file[theInstruction->op1].data;
               res_mul[idx].ins.op2 = reg_file[theInstruction->op2].data;
               res_mul[idx].ready = 1;
            }else
            {
               if(res_mul[idx].src1_tag != 0)
               {res_mul[idx].ins.op1 = 0;}
               else
               {res_mul[idx].ins.op1 = reg_file[theInstruction->op1].data;}

               if(res_mul[idx].src2_tag != 0)
               {res_mul[idx].ins.op2 = 0;}
               else
               {res_mul[idx].ins.op2 = reg_file[theInstruction->op2].data;}

               res_mul[idx].ready = 0;
            }
            res_mul[idx].life = 0;
            reg_file[theInstruction->dest].tag = idx + ADD_RES_NUM +1;

            return 1;
         }
         else
            return 0;
      }
      break;
   }
   return 1;
}

int checkDone(int registerImage[NUM_REGISTERS]) {
   uint32_t count = 0;
	show_res_entries(res_add,ADD_RES_NUM);
	show_res_entries(res_mul,MUL_RES_NUM);
   for(count = 0; count < ADD_RES_NUM;count++)
   {
      if(res_add[count].busy)
      {
         printf("instruction in adder reservation station\n");
         return 0;
      }
   }
   for(count = 0; count < MUL_RES_NUM;count++)
   {
      if(res_mul[count].busy)
      {
         printf("instruction in mul reservation station\n");
         return 0;
      }
   }

   int i;

   for (i=0; i < NUM_REGISTERS; i++) {
      registerImage[i] = reg_file[i].data;
   }
   return (1);
}

int32_t get_available_slot(reservation_entry_t * res)
{
   uint32_t num_of_entries = 0;
   if(res == res_add)
      {num_of_entries = ADD_RES_NUM;}
   else if(res == res_mul)
      {num_of_entries = MUL_RES_NUM;}
   else
   {fprintf(stderr,"Invalid pointer for reservation station %p\n",res);}
   uint32_t count = 0;
   for(count = 0; count < num_of_entries;count++)
   {
      if(res[count].busy == 0)
         return count;
   }
   return -1;
}

int32_t get_next_ins_idx(reservation_entry_t * res)
{
   uint32_t idx = 0;
   uint32_t life_max = 0;
   uint32_t max_idx = -1;
   uint32_t num_of_entries = 0;
   if(res == res_add)
      {num_of_entries = ADD_RES_NUM;}
   else if(res == res_mul)
      {num_of_entries = MUL_RES_NUM;}
   else
   {fprintf(stderr,"Invalid pointer for reservation station %p\n",res);}
   
   uint32_t count = 0;
   for(count = 0; count < num_of_entries;count++)
   {
      if(res[count].busy && res[count].ready)
      {
         if(res[count].life > life_max)
         {
            life_max = res[count].life;
            max_idx = count;
         }
         else
            res[count].life++;
      }else if(res[count].busy)
		res[count].life++;
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

   fscanf(fd,"%d\n",add_res_num);
   fscanf(fd,"%d\n",mul_res_num);
   printf("Adder reservation slots %d\nMultiplier reservation slots %d\n",*add_res_num,*mul_res_num);
   fclose(fd);
	return ;
}

void update_res(reservation_entry_t * res, uint32_t num, writeResult_t * writeResult)
{
   int tag = writeResult->tag;
   uint32_t idx = 0;
   for(idx = 0;idx < num;idx++)
   {
	if(writeResult->tag > ADD_RES_NUM && res == res_mul)
		if((writeResult->tag - ADD_RES_NUM - 1) == idx)
			res_mul[idx].busy = 0;
	if(writeResult->tag <= ADD_RES_NUM && res == res_add)
		if((writeResult->tag - 1) == idx)
			res_add[idx].busy = 0;
      if(res[idx].busy && (res[idx].src1_tag == tag || res[idx].src2_tag == tag))
      {
	if(res == res_add)
		printf("tag matched happen for res_add %d\n",idx);
        else
		printf("tag mathced happen for res_mul %d\n",idx); 
	if(res[idx].src1_tag == tag)
         {
            res[idx].src1_tag = 0;
            res[idx].ins.op1 = writeResult->value;
         }

         if(res[idx].src2_tag == tag)
         {
            res[idx].src2_tag = 0;
            res[idx].ins.op2 = writeResult->value;
         }

         if(res[idx].src1_tag == 0 && res[idx].src2_tag == 0)
            res[idx].ready = 1;
      }
   }
}

void show_res_entries(reservation_entry_t *res,uint32_t num)
{
   uint32_t idx = 0;
   printf("busy\t|ready\t|Type\t|dest\t|op1\t|op2\t|tag1\t|tag2\t|life\n");
   for(idx = 0;idx < num;idx++)
   {
      printf("%u\t|%u\t|%u\t|%d\t|%d\t|%d\t|%d\t|%d\t|%d\n",\
         res[idx].busy,\
         res[idx].ready,\
         res[idx].ins.instructionType,\
         res[idx].ins.dest,\
         res[idx].ins.op1,\
         res[idx].ins.op2,\
         res[idx].src1_tag,\
         res[idx].src2_tag,\
         res[idx].life);
   }
}
