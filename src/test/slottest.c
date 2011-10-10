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

#include "shared/parseargs.h"
#include "shared/test.h"
#include <ramalloc/ramalloc.h>
#include <ramalloc/algn.h>
#include <ramalloc/misc.h>
#include <ramalloc/thread.h>
#include <ramalloc/barrier.h>
#include <ramalloc/stdint.h>
#include <ramalloc/annotate.h>
#include <ramalloc/cast.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#define DEFAULT_ALLOCATION_COUNT 1024 * 100
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

typedef struct extra
{
   ramslot_pool_t e_thepool;
} extra_t;

#define ALLOCATION_SIZE sizeof(slot_t)

static ram_reply_t main2(int argc, char *argv[]);
static ram_reply_t initdefaults(ramtest_params_t *params_arg);
static ram_reply_t runtest(const ramtest_params_t *params_arg);
static ram_reply_t runtest2(const ramtest_params_t *params_arg,
      extra_t *extra_arg);
static ram_reply_t getpool(ramslot_pool_t **pool_arg, void *extra_arg,
      size_t threadidx_arg);
static ram_reply_t acquire(ramtest_allocdesc_t *desc_arg,
      size_t size_arg, void *extra_arg, size_t threadidx_arg);
static ram_reply_t release(ramtest_allocdesc_t *desc_arg);
static ram_reply_t query(void **pool_arg, size_t *size_arg,
      void *ptr_arg, void *extra_arg);
static ram_reply_t flush(void *extra_arg, size_t threadidx_arg);
static ram_reply_t check(void *extra_arg, size_t threadidx_arg);
static ram_reply_t mknode(ramslot_node_t **node_arg, void **slots_arg,
      ramslot_pool_t *pool_arg);
static ram_reply_t rmnode(ramslot_node_t *node_arg);
static ram_reply_t initslot(void *slot_arg, ramslot_node_t *node_arg);

static ramsig_signature_t thesig;

int main(int argc, char *argv[])
{
   ram_reply_t e = RAM_REPLY_INSANE;
   size_t unused = 0;

   e = main2(argc, argv);
   if (RAM_REPLY_OK != e)
      RAM_FAIL_TRAP(ramtest_fprintf(&unused, stderr, "fail (%d).", e));
   if (RAM_REPLY_INPUTFAIL == e)
   {
      usage(e, argc, argv);
      ram_fail_panic("unreachable code.");
      return RAM_REPLY_INSANE;
   }
   else
      return e;
}

ram_reply_t main2(int argc, char *argv[])
{
   ramtest_params_t testparams;
   ram_reply_t e = RAM_REPLY_INSANE;

   RAM_FAIL_TRAP(ram_initialize(NULL, NULL));

   RAM_FAIL_TRAP(initdefaults(&testparams));
   e = parseargs(&testparams, argc, argv);
   switch (e)
   {
   default:
      RAM_FAIL_TRAP(e);
   case RAM_REPLY_OK:
      break;
   case RAM_REPLY_INPUTFAIL:
      return e;
   }

   e = runtest(&testparams);
   switch (e)
   {
   default:
      RAM_FAIL_TRAP(e);
   case RAM_REPLY_OK:
      break;
   case RAM_REPLY_INPUTFAIL:
      return e;
   }

   return RAM_REPLY_OK;
}

ram_reply_t initdefaults(ramtest_params_t *params_arg)
{
   RAM_FAIL_NOTNULL(params_arg);
   memset(params_arg, 0, sizeof(*params_arg));

   params_arg->ramtestp_alloccount = DEFAULT_ALLOCATION_COUNT;
   /* the slot pool doesn't support parallelized access. */
   params_arg->ramtestp_threadcount = 1;
   /* the slot pool doesn't support detection of foreign pointers. */
   params_arg->ramtestp_mallocchance = 0;
   params_arg->ramtestp_minsize = ALLOCATION_SIZE;
   params_arg->ramtestp_maxsize = ALLOCATION_SIZE;

   return RAM_REPLY_OK;
}

ram_reply_t getpool(ramslot_pool_t **pool_arg, void *extra_arg,
      size_t threadidx_arg)
{
   extra_t *x = NULL;

   RAM_FAIL_NOTNULL(pool_arg);
   *pool_arg = NULL;
   RAM_FAIL_NOTNULL(extra_arg);
   x = (extra_t *)extra_arg;
   RAMANNOTATE_UNUSEDARG(threadidx_arg);

   *pool_arg = &x->e_thepool;
   return RAM_REPLY_OK;
}

ram_reply_t acquire(ramtest_allocdesc_t *desc_arg,
      size_t size_arg, void *extra_arg, size_t threadidx_arg)
{
   ramslot_pool_t *pool = NULL;
   void *p = NULL;

   RAM_FAIL_NOTNULL(desc_arg);
   memset(desc_arg, 0, sizeof(*desc_arg));
   RAM_FAIL_NOTZERO(size_arg);

   RAM_FAIL_TRAP(getpool(&pool, extra_arg, threadidx_arg));
   RAM_FAIL_TRAP(ramslot_acquire(&p, pool));
   desc_arg->ramtestad_ptr = (char *)p;
   desc_arg->ramtestad_pool = pool;
   desc_arg->ramtestad_sz = size_arg;

   return RAM_REPLY_OK;
}

ram_reply_t release(ramtest_allocdesc_t *desc_arg)
{
   slot_t *s = NULL;

   RAM_FAIL_NOTNULL(desc_arg);

   s = (slot_t *)desc_arg->ramtestad_ptr;
   RAM_FAIL_EXPECT(RAM_REPLY_CORRUPT, 0 == RAMSIG_CMP(s->s_sig, thesig));
   RAM_FAIL_EXPECT(RAM_REPLY_CORRUPT, s->s_value == s - s->s_node->n_slots);
   RAM_FAIL_TRAP(ramslot_release(s, &s->s_node->n_slotnode));

   return RAM_REPLY_OK;
}

ram_reply_t query(void **pool_arg, size_t *size_arg, void *ptr_arg,
      void *extra_arg)
{
   extra_t *x = NULL;

   RAM_FAIL_NOTNULL(pool_arg);
   *pool_arg = NULL;
   RAM_FAIL_NOTNULL(ptr_arg);
   RAM_FAIL_NOTNULL(extra_arg);

   x = (extra_t *)extra_arg;
   /* slot pools don't support the query option, so i emulate it by
    * returning the pointer stored in the extra info. */
   *pool_arg = &x->e_thepool;
   RAM_FAIL_TRAP(ramslot_getgranularity(size_arg, &x->e_thepool));
   return RAM_REPLY_OK;
}

ram_reply_t flush(void *extra_arg, size_t threadidx_arg)
{
   RAMANNOTATE_UNUSEDARG(extra_arg);
   RAMANNOTATE_UNUSEDARG(threadidx_arg);
   /* slot pools don't support the flush operation. */
   return RAM_REPLY_OK;
}

ram_reply_t check(void *extra_arg, size_t threadidx_arg)
{
   ramslot_pool_t *pool = NULL;

   RAM_FAIL_TRAP(getpool(&pool, extra_arg, threadidx_arg));
   RAM_FAIL_TRAP(ramslot_chkpool(pool));

   return RAM_REPLY_OK;
}

ram_reply_t runtest(const ramtest_params_t *params_arg)
{
   ram_reply_t e = RAM_REPLY_INSANE;
   extra_t x;

   RAM_FAIL_NOTNULL(params_arg);

   e = runtest2(params_arg, &x);

   return e;
}

ram_reply_t runtest2(const ramtest_params_t *params_arg,
      extra_t *extra_arg)
{
   ramtest_params_t testparams = {0};
   size_t unused = 0;

   testparams = *params_arg;

   if (ALLOCATION_SIZE != testparams.ramtestp_maxsize ||
         ALLOCATION_SIZE != testparams.ramtestp_minsize)
   {
      RAM_FAIL_TRAP(ramtest_fprintf(&unused, stderr,
            "warning: this test doesn't support customized sizes. i will "
            "use the predetermined size (%zu bytes) for all "
            "allocations.\n", ALLOCATION_SIZE));
      testparams.ramtestp_maxsize = ALLOCATION_SIZE;
      testparams.ramtestp_minsize = ALLOCATION_SIZE;
   }
   /* the pgpool doesn't support multi-threaded access. */
   if (testparams.ramtestp_threadcount > 1)
   {
      RAM_FAIL_TRAP(ramtest_fprintf(&unused, stderr,
            "the --parallelize option is not supported in this test.\n"));
      return RAM_REPLY_INPUTFAIL;
   }
   /* the pgpool is unable to detect whether it allocated a given
    * object. */
   if (testparams.ramtestp_mallocchance != 0)
   {
      RAM_FAIL_TRAP(ramtest_fprintf(&unused, stderr,
            "the --mallocchance option is not supported in this test.\n"));
      return RAM_REPLY_INPUTFAIL;
   }

   testparams.ramtestp_nofill = 1;

   testparams.ramtestp_extra = extra_arg;
   testparams.ramtestp_acquire = &acquire;
   testparams.ramtestp_release = &release;
   testparams.ramtestp_query = &query;
   testparams.ramtestp_flush = &flush;
   testparams.ramtestp_check = &check;

   RAM_FAIL_TRAP(ramsig_init(&thesig, "TEST"));
   RAM_FAIL_TRAP(ramslot_mkpool(&extra_arg->e_thepool, ALLOCATION_SIZE,
         NODE_CAPACITY, &mknode, &rmnode, &initslot));

   RAM_FAIL_TRAP(ramtest_test(&testparams));

   return RAM_REPLY_OK;
}

ram_reply_t mknode(ramslot_node_t **node_arg, void **slots_arg,
      ramslot_pool_t *pool_arg)
{
   node_t *node = NULL;

   RAM_FAIL_NOTNULL(node_arg);
   *node_arg = NULL;
   RAM_FAIL_NOTNULL(slots_arg);
   *slots_arg = NULL;
   RAM_FAIL_NOTNULL(pool_arg);

   node = (node_t *)malloc(sizeof(node_t));
   if (node)
   {
      *slots_arg = node->n_slots;
      *node_arg = &node->n_slotnode;
      return RAM_REPLY_OK;
   }
   else
      return RAM_REPLY_CRTFAIL;
}

ram_reply_t rmnode(ramslot_node_t *node_arg)
{
   RAM_FAIL_NOTNULL(node_arg);

   free(node_arg);

   return RAM_REPLY_OK;
}

ram_reply_t initslot(void *slot_arg, ramslot_node_t *node_arg)
{
   node_t *node = NULL;
   slot_t *slot = NULL;

   RAM_FAIL_NOTNULL(slot_arg);
   RAM_FAIL_NOTNULL(node_arg);

   node = RAM_CAST_STRUCTBASE(node_t, n_slotnode, node_arg);
   slot = (slot_t *)slot_arg;
   slot->s_sig = thesig;
   slot->s_node = node;
   slot->s_value = slot - node->n_slots;

   return RAM_REPLY_OK;
}
