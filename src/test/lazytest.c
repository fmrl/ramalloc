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
#include <ramalloc/lazy.h>
#include <ramalloc/misc.h>
#include <ramalloc/thread.h>
#include <ramalloc/barrier.h>
#include <ramalloc/test.h>
#include <ramalloc/stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#define ALLOCATION_COUNT 1024 * 100
#define ALLOCATION_MINSIZE 4
#define ALLOCATION_MAXSIZE 100
#define RECLAIMRATIO 2
#define THREAD_COUNT 4
#define MALLOC_PERCENTAGE 30

typedef struct extra
{
   ramlazy_pool_t *e_pools;
   size_t e_poolcount;
} extra_t;

static ramfail_status_t newtest(size_t threadcount_arg);
static ramfail_status_t newtest2(size_t threadcount_arg,
      extra_t *extra_arg);
static ramfail_status_t getpool(ramlazy_pool_t **pool_arg, void *extra_arg,
      int threadidx_arg);
static ramfail_status_t acquire(ramtest_allocdesc_t *desc_arg,
      size_t size_arg, void *extra_arg, int threadidx_arg);
static ramfail_status_t release(ramtest_allocdesc_t *desc_arg);
static ramfail_status_t query(void **pool_arg, size_t *size_arg,
      void *ptr_arg);
static ramfail_status_t flush(void *extra_arg, int threadidx_arg);
static ramfail_status_t check(void *extra_arg, int threadidx_arg);

int main()
{
   RAMFAIL_CONFIRM(-1, RAMFAIL_OK == ramalloc_initialize(NULL, NULL));
   RAMFAIL_CONFIRM(3, RAMFAIL_OK == newtest(THREAD_COUNT));

   return 0;
}

ramfail_status_t getpool(ramlazy_pool_t **pool_arg, void *extra_arg,
      int threadidx_arg)
{
   extra_t *x = NULL;

   RAMFAIL_DISALLOWZ(pool_arg);
   *pool_arg = NULL;
   RAMFAIL_DISALLOWZ(extra_arg);
   x = (extra_t *)extra_arg;
   RAMFAIL_CONFIRM(RAMFAIL_RANGE, threadidx_arg >= 0);
   RAMFAIL_CONFIRM(RAMFAIL_RANGE, threadidx_arg < x->e_poolcount);

   *pool_arg = &x->e_pools[threadidx_arg];
   return RAMFAIL_OK;
}

ramfail_status_t acquire(ramtest_allocdesc_t *desc_arg,
      size_t size_arg, void *extra_arg, int threadidx_arg)
{
   ramlazy_pool_t *pool = NULL;
   void *p = NULL;

   RAMFAIL_DISALLOWZ(desc_arg);
   memset(desc_arg, 0, sizeof(*desc_arg));
   RAMFAIL_DISALLOWZ(size_arg);

   RAMFAIL_RETURN(getpool(&pool, extra_arg, threadidx_arg));
   RAMFAIL_RETURN(ramlazy_acquire(&p, pool, size_arg));
   desc_arg->ramtestad_ptr = (char *)p;
   desc_arg->ramtestad_pool = pool;
   desc_arg->ramtestad_sz = size_arg;

   return RAMFAIL_OK;
}

ramfail_status_t release(ramtest_allocdesc_t *desc_arg)
{
   RAMFAIL_DISALLOWZ(desc_arg);

   RAMFAIL_RETURN(ramlazy_release(desc_arg->ramtestad_ptr));

   return RAMFAIL_OK;
}

ramfail_status_t query(void **pool_arg, size_t *size_arg, void *ptr_arg)
{
   ramlazy_pool_t *pool = NULL;
   ramfail_status_t e = RAMFAIL_INSANE;

   RAMFAIL_DISALLOWZ(pool_arg);
   *pool_arg = NULL;

   e = ramlazy_query(&pool, size_arg, ptr_arg);
   switch (e)
   {
   default:
      RAMFAIL_RETURN(e);
      return RAMFAIL_INSANE;
   case RAMFAIL_OK:
      *pool_arg = pool;
      return RAMFAIL_OK;
   case RAMFAIL_NOTFOUND:
      return e;
   }

   return RAMFAIL_OK;
}

ramfail_status_t flush(void *extra_arg, int threadidx_arg)
{
   ramlazy_pool_t *pool = NULL;

   RAMFAIL_RETURN(getpool(&pool, extra_arg, threadidx_arg));
   RAMFAIL_RETURN(ramlazy_flush(pool));

   return RAMFAIL_OK;
}

ramfail_status_t check(void *extra_arg, int threadidx_arg)
{
   ramlazy_pool_t *pool = NULL;

   RAMFAIL_RETURN(getpool(&pool, extra_arg, threadidx_arg));
   RAMFAIL_RETURN(ramlazy_chkpool(pool));

   return RAMFAIL_OK;
}

ramfail_status_t newtest(size_t threadcount_arg)
{
   ramfail_status_t e = RAMFAIL_INSANE;
   extra_t x = {0};

   RAMFAIL_DISALLOWZ(threadcount_arg);

   x.e_pools = calloc(threadcount_arg, sizeof(*x.e_pools));
   x.e_poolcount = threadcount_arg;
   e = newtest2(threadcount_arg, &x);
   free(x.e_pools);
   RAMFAIL_RETURN(e);

   return RAMFAIL_OK;
}

ramfail_status_t newtest2(size_t threadcount_arg, extra_t *extra_arg)
{
   size_t i = 0;
   ramtest_params_t tp = {0};

   for (i = 0; i < threadcount_arg; ++i)
   {
      RAMFAIL_RETURN(ramlazy_mkpool(&extra_arg->e_pools[i],
            RAMOPT_DEFAULTAPPETITE, RECLAIMRATIO));
   }

   tp.ramtestp_alloccount = ALLOCATION_COUNT;
   tp.ramtestp_threadcount = THREAD_COUNT;
   /* i would like 30% of the allocations to come from malloc() instead
    * of the allocator i'm testing. */
   tp.ramtestp_mallocchance = MALLOC_PERCENTAGE;
   /* i'm going to test a narrow band of allocations to exercise the
    * allocator as deeply as possible. */
   tp.ramtestp_minsize = ALLOCATION_MINSIZE;
   tp.ramtestp_maxsize = ALLOCATION_MAXSIZE;
   tp.ramtestp_extra = extra_arg;

   tp.ramtestp_acquire = &acquire;
   tp.ramtestp_release = &release;
   tp.ramtestp_query = &query;
   tp.ramtestp_flush = &flush;
   tp.ramtestp_check = &check;

   RAMFAIL_RETURN(ramtest_test(&tp));

   return RAMFAIL_OK;
}

