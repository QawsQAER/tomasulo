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

#define CONFIG_FILE "config.default"
//the reservation station entry
typedef struct{
   unsigned char busy;//indicates whether the slot is occupied
   unsigned char ready;//indicates whether the instruction is ready to by executed
   unsigned char executed;
   instruction_t ins;
   unsigned short src1_tag;
   unsigned short src2_tag;
   unsigned life;//indicates how long this slot is occupied by the current instruction
} reservation_entry_t;

//the register entry
typedef struct 
{
   /*
      0 means there's no instruction that is writing to this register.
      [1, ADD_RES_NUM] means instruction in add reservation stations is writing to this register
      [ADD_RES_NUM + 1, ADD_RES_NUM + MUL_RES_NUM + 1] means instruction in mul reservation stations is writing to this register
   */
   unsigned short tag;
   int32_t data;
} register_entry;

//the reservation station
unsigned ADD_RES_NUM = 3;
unsigned MUL_RES_NUM = 2;
reservation_entry_t *res_add;
reservation_entry_t *res_mul;
register_entry *reg_file;

/*
   This function will retrieve the index of the first available slot
   in the reservation station pointed by the res pointer.
*/
int32_t get_available_slot(reservation_entry_t * res);

/*
   this function will retrieve the index of the reservation station
   for the next-to-be-executed reservation station entry 
*/

int32_t get_next_ins_idx(reservation_entry_t * res);

/*
   this function will fopen() the configuration, and read in the config about
   the number of slots of reservation stations.
*/
void my_get_config(unsigned * add_res_num, unsigned * mul_res_num);

void update_res(reservation_entry_t *res, unsigned num, writeResult_t *theResult);

void show_res_entries(reservation_entry_t *res, unsigned num);

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
   #ifdef DEBUG
      printf("insturction with tag %d is finished\n",theResult->tag);
   #endif

   update_res(res_add,ADD_RES_NUM,theResult);
   update_res(res_mul,MUL_RES_NUM,theResult);
   unsigned idx = 0;
   for(idx = 0;idx < NUM_REGISTERS;idx++)
   {
      if(reg_file[idx].tag == theResult->tag)
      {
         reg_file[idx].tag = 0;
         reg_file[idx].data = theResult->value;
         #ifdef DEBUG
            printf("Writing %d into reg_file[%d]\n",theResult->value,idx);
         #endif
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
      if((idx = get_next_ins_idx(res_add)) < 0)
         return 0;
      executeRequest->tag = idx + 1;
      executeRequest->op1 = res_add[idx].ins.op1;
      executeRequest->op2 = res_add[idx].ins.op2;
      res_add[idx].executed = 1;
      res_add[idx].life = 0;
      #ifdef DEBUG
         printf("execute %d in res_add, propagating tag %d\n",idx,executeRequest->tag);
         show_res_entries(res_add,ADD_RES_NUM);
      #endif
   }
   else if(mathOpType == mult)
   {
      if((idx = get_next_ins_idx(res_mul)) < 0)
         return 0;
      executeRequest->tag = idx + ADD_RES_NUM + 1;
      executeRequest->op1 = res_mul[idx].ins.op1;
      executeRequest->op2 = res_mul[idx].ins.op2;
      res_mul[idx].executed = 1;
      res_mul[idx].life = 0;
      #ifdef DEBUG
         printf("execute %d in res_mul, propagating tag %d\n",idx,executeRequest->tag);
         show_res_entries(res_mul,MUL_RES_NUM);
      #endif
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
   #ifdef DEBUG
      printf("%d %d %d %d\n", theInstruction->instructionType, theInstruction->dest, theInstruction->op1, theInstruction->op2);
   #endif
   int32_t idx = 0;
   switch(theInstruction->instructionType)
   {
      case addImm:
      {
         if((idx = get_available_slot(res_add)) >= 0) 
         {
            //issuing theInstruction to res_add[idx]
            #ifdef DEBUG
               printf("issuing theInstruction to res_add[%d]\n",idx);
            #endif
            res_add[idx].busy = 1;
            res_add[idx].executed = 0;
            res_add[idx].ins.instructionType = theInstruction->instructionType;
            res_add[idx].ins.dest = theInstruction->dest;
            res_add[idx].src1_tag = reg_file[theInstruction->op1].tag;
            res_add[idx].src2_tag = 0;
            if(res_add[idx].src1_tag == 0)
            {
               //get the data immediately, and the instruction is now ready to be executed
               res_add[idx].ins.op1 = reg_file[theInstruction->op1].data;
               res_add[idx].ready = 1;
            }
            else
            {
               //case that the this instruction should be waiting for other instruction's output
               res_add[idx].ins.op1 = 0;
               res_add[idx].ready = 0;
            }
            res_add[idx].ins.op2 = theInstruction->op2;
            res_add[idx].life = 1;

            //update the destination register.
            reg_file[theInstruction->dest].tag = idx + 1;
            #ifdef DEBUG
               show_res_entries(res_add,ADD_RES_NUM);
            #endif
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
            #ifdef DEBUG
               printf("issuing theInstruction to res_add[%d]\n",idx);
            #endif
            res_add[idx].busy = 1;
            res_add[idx].executed = 0;
            res_add[idx].ins.instructionType = theInstruction->instructionType;
            res_add[idx].ins.dest = theInstruction->dest;
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
            res_add[idx].life = 1;
            //update the destination register.
            reg_file[theInstruction->dest].tag = idx + 1;
            #ifdef DEBUG
               show_res_entries(res_add,ADD_RES_NUM);
            #endif
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
            #ifdef DEBUG
               printf("issuing theInstruction to res_mul[%d]\n",idx);
            #endif
            res_mul[idx].busy = 1;
            res_mul[idx].executed = 0;
            res_mul[idx].ins.instructionType = theInstruction->instructionType;
            res_mul[idx].ins.dest = theInstruction->dest;
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
            res_mul[idx].life = 1;

            //update the destination register.
            reg_file[theInstruction->dest].tag = idx + ADD_RES_NUM +1;
            #ifdef DEBUG
               show_res_entries(res_mul,MUL_RES_NUM);
            #endif
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
            #ifdef DEBUG
               printf("issuing theInstruction to res_mul[%d]\n",idx);
            #endif
            res_mul[idx].busy = 1;
            res_mul[idx].executed = 0;
            res_mul[idx].ins.instructionType = theInstruction->instructionType;
            res_mul[idx].ins.dest = theInstruction->dest;
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
            res_mul[idx].life = 1;
            reg_file[theInstruction->dest].tag = idx + ADD_RES_NUM +1;
            #ifdef DEBUG
               show_res_entries(res_mul,MUL_RES_NUM);
            #endif
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
   unsigned count = 0;
   for(count = 0; count < ADD_RES_NUM;count++)
   {
      if(res_add[count].busy)
      {
         //printf("instruction in adder reservation station\n");
         return 0;
      }
   }
   for(count = 0; count < MUL_RES_NUM;count++)
   {
      if(res_mul[count].busy)
      {
         //printf("instruction in mul reservation station\n");
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
   unsigned num_of_entries = 0;
   if(res == res_add)
      {num_of_entries = ADD_RES_NUM;}
   else if(res == res_mul)
      {num_of_entries = MUL_RES_NUM;}
   else
   {fprintf(stderr,"Invalid pointer for reservation station %p\n",res);}
   unsigned count = 0;
   for(count = 0; count < num_of_entries;count++)
   {
      if(res[count].busy == 0)
         return count;
   }
   return -1;
}

int32_t get_next_ins_idx(reservation_entry_t * res)
{
   unsigned idx = 0;
   unsigned life_max = 0;
   int32_t max_idx = -1;
   unsigned num_of_entries = 0;
   if(res == res_add)
      {num_of_entries = ADD_RES_NUM;}
   else if(res == res_mul)
      {num_of_entries = MUL_RES_NUM;}
   else
   {fprintf(stderr,"Invalid pointer for reservation station %p\n",res);}
   
   unsigned count = 0;
   for(count = 0; count < num_of_entries;count++)
   {
      if(res[count].busy && res[count].ready && (res[count].executed == 0))
      {
         if(res[count].life > life_max)
         {
            life_max = res[count].life++;
            max_idx = count;
         }
         else
            res[count].life++;
      }else if(res[count].busy && (res[count].executed == 0))
		 res[count].life++;
   }
   return max_idx;
}

void my_get_config(unsigned * add_res_num, unsigned * mul_res_num)
{
	FILE *fd = fopen(CONFIG_FILE,"r");
	if(fd == NULL)
	{
		fprintf(stderr,"Cannot open %s, please check the path",CONFIG_FILE);
		exit(0);
	}

   fscanf(fd,"%d\n",add_res_num);
   fscanf(fd,"%d\n",mul_res_num);
   //printf("Adder reservation slots %d\nMultiplier reservation slots %d\n",*add_res_num,*mul_res_num);
   fclose(fd);
	return ;
}

void update_res(reservation_entry_t * res, unsigned num, writeResult_t * writeResult)
{
   int tag = writeResult->tag;
   unsigned idx = 0;
   for(idx = 0;idx < num;idx++)
   {
      
      if(writeResult->tag <= ADD_RES_NUM && res == res_add && res[idx].executed)
         if((writeResult->tag - 1) == idx)
			   res[idx].busy = 0;

      if(writeResult->tag > ADD_RES_NUM && res == res_mul && res[idx].executed)
         if((writeResult->tag - ADD_RES_NUM - 1) == idx)
            res[idx].busy = 0;
      
      if(res[idx].busy && (res[idx].src1_tag == tag || res[idx].src2_tag == tag) && (res[idx].ready == 0))
      {
         #ifdef DEBUG
         if(res == res_add)
            printf("tag matched happen for res_add %d\n",idx);
         else
            printf("tag mathced happen for res_mul %d\n",idx); 
         #endif

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

void show_res_entries(reservation_entry_t *res,unsigned num)
{
   unsigned idx = 0;
   printf("busy\t|ready\t|exe\t|Type\t|dest\t|op1\t|op2\t|tag1\t|tag2\t|life\n");
   for(idx = 0;idx < num;idx++)
   {
      printf("%4u\t|%5u\t|%3u\t|%4u\t|%4d\t|%3d\t|%3d\t|%4d\t|%4d\t|%4d\n",\
         res[idx].busy,\
         res[idx].ready,\
         res[idx].executed,\
         res[idx].ins.instructionType,\
         res[idx].ins.dest,\
         res[idx].ins.op1,\
         res[idx].ins.op2,\
         res[idx].src1_tag,\
         res[idx].src2_tag,\
         res[idx].life);
   }
}
