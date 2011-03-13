/* ex: set softtabstop=3 shiftwidth=3 expandtab: */

/* This file is part of the *ramalloc* project at <http://fmrl.org>.
 * Copyright (c) 2011, Michael Lowell Roberts.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are 
 * met: 
 *
 *  * Redistributions of source code must retain the above copyright 
 *  notice, this list of conditions and the following disclaimer. 
 *
 *  * Redistributions in binary form must reproduce the above copyright 
 *  notice, this list of conditions and the following disclaimer in the 
 *  documentation and/or other materials provided with the distribution.
 * 
 *  * Neither the name of the copyright holder nor the names of 
 *  contributors may be used to endorse or promote products derived 
 *  from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS 
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER 
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#include <ramalloc/ramalloc.h>
#include <ramalloc/slot.h>
#include <ramalloc/misc.h>
#include <ramalloc/sig.h>
#include <ramalloc/meta.h>
#include <ramalloc/test.h>
#include <ramalloc/sys/stdint.h>
#include <stdlib.h>

#define ALLOCATION_COUNT 10000
#define NODE_CAPACITY 10

struct node;

typedef struct slot
{
   ramsig_signature_t s_sig;
   struct node *s_node;
   int32_t s_value;
} slot_t;

typedef struct node
{
   ramslot_node_t n_slotnode;
   slot_t n_slots[NODE_CAPACITY];
} node_t;

#define ALLOCATION_SIZE sizeof(slot_t)
#define NODE_SIZE (ALLOCATION_SIZE * NODE_CAPACITY)

static ramfail_status_t sequentialtest();
static ramfail_status_t randomtest();
static ramfail_status_t mknode(ramslot_node_t **node_arg, void **slots_arg, ramslot_pool_t *pool_arg);
static ramfail_status_t rmnode(ramslot_node_t *node_arg, ramslot_pool_t *pool_arg);
static ramfail_status_t initslot(void *slot_arg, ramslot_node_t *node_arg);
static ramfail_status_t release(void *ptr_arg);


static ramsig_signature_t thesig;

int main()
{
   RAMFAIL_CONFIRM(-1, RAMFAIL_OK == ramalloc_initialize(NULL, NULL));
   RAMFAIL_CONFIRM(1, RAMFAIL_OK == sequentialtest());
   RAMFAIL_CONFIRM(2, RAMFAIL_OK == randomtest());

   return 0;
}

ramfail_status_t sequentialtest()
{
   void *(p[ALLOCATION_COUNT]) = {0};
   size_t i = 0;
   ramslot_pool_t pool;

   RAMFAIL_RETURN(ramsig_init(&thesig, "SEQU"));
   RAMFAIL_RETURN(ramslot_mkpool(&pool, ALLOCATION_SIZE, NODE_CAPACITY, &mknode, &rmnode, &initslot));
   RAMFAIL_RETURN(ramslot_chkpool(&pool));
   for (i = 0; i < ALLOCATION_COUNT; ++i)
   {
      RAMFAIL_RETURN(ramslot_acquire(&p[i], &pool));
      RAMFAIL_RETURN(ramslot_chkpool(&pool));
   }

   for (i = 0; i < ALLOCATION_COUNT; ++i)
   {
      RAMFAIL_RETURN(release(p[i]));
      RAMFAIL_RETURN(ramslot_chkpool(&pool));
   }

   return RAMFAIL_OK;
}

ramfail_status_t randomtest()
{
   void *(p[ALLOCATION_COUNT]) = {0};
   size_t idx[ALLOCATION_COUNT] = {0};
   size_t i = 0;
   ramslot_pool_t pool;

   srand(0/*(unsigned int)time(0)*/);

   for (i = 0; i < ALLOCATION_COUNT; ++i)
      idx[i] = i;
   RAMFAIL_RETURN(ramtest_shuffle(idx, sizeof(idx[0]), ALLOCATION_COUNT));

   RAMFAIL_RETURN(ramsig_init(&thesig, "RAND"));
   RAMFAIL_RETURN(ramslot_mkpool(&pool, ALLOCATION_SIZE, NODE_CAPACITY, &mknode, &rmnode, &initslot));
   RAMFAIL_RETURN(ramslot_chkpool(&pool));
   for (i = 0; i < ALLOCATION_COUNT; ++i)
   {
      RAMFAIL_RETURN(ramslot_acquire(&p[i], &pool));
      RAMFAIL_RETURN(ramslot_chkpool(&pool));
   }

   for (i = 0; i < ALLOCATION_COUNT; ++i)
   {
      RAMFAIL_RETURN(release(p[i]));
      RAMFAIL_RETURN(ramslot_chkpool(&pool));
   }

   return RAMFAIL_OK;
}



ramfail_status_t mknode(ramslot_node_t **node_arg, void **slots_arg, ramslot_pool_t *pool_arg)
{
   node_t *node = NULL;

   RAMFAIL_DISALLOWZ(node_arg);
   *node_arg = NULL;
   RAMFAIL_DISALLOWZ(slots_arg);
   *slots_arg = NULL;
   RAMFAIL_DISALLOWZ(pool_arg);

   node = (node_t *)malloc(sizeof(node_t));
   if (node)
   {
      *slots_arg = node->n_slots;
      *node_arg = &node->n_slotnode;
      return RAMFAIL_OK;
   }
   else
      return RAMFAIL_CRT;
}

ramfail_status_t rmnode(ramslot_node_t *node_arg, ramslot_pool_t *pool_arg)
{
   RAMFAIL_DISALLOWZ(node_arg);
   RAMFAIL_DISALLOWZ(pool_arg);

   free(node_arg);

   return RAMFAIL_OK;
}

ramfail_status_t initslot(void *slot_arg, ramslot_node_t *node_arg)
{
   node_t *node = NULL;
   slot_t *slot = NULL;

   RAMFAIL_DISALLOWZ(slot_arg);
   RAMFAIL_DISALLOWZ(node_arg);

   RAMMETA_BACKCAST(node, node_t, n_slotnode, node_arg);
   slot = (slot_t *)slot_arg;
   slot->s_sig = thesig;
   slot->s_node = node;
   slot->s_value = slot - node->n_slots;

   return RAMFAIL_OK;
}

ramfail_status_t release(void *ptr_arg)
{
   slot_t *s = NULL;

   RAMFAIL_DISALLOWZ(ptr_arg);

   s = (slot_t *)ptr_arg;
   RAMFAIL_CONFIRM(RAMFAIL_CORRUPT, 0 == RAMSIG_CMP(s->s_sig, thesig));
   RAMFAIL_CONFIRM(RAMFAIL_CORRUPT, s->s_value == s - s->s_node->n_slots);
   RAMFAIL_RETURN(ramslot_release(s, &s->s_node->n_slotnode));

   return RAMFAIL_OK;
}
