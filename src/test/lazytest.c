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

typedef struct allocinfo
{
   char *ptr;
   size_t sz;
   ramlazy_pool_t *pool;
   rammtx_mutex_t mtx;
} allocinfo_t;

allocinfo_t allocinfos[ALLOCATION_COUNT] = {0};
size_t sequence[ALLOCATION_COUNT * 2] = {0};
ramlazy_pool_t pools[THREAD_COUNT] = {0};
ramthread_thread_t threads[THREAD_COUNT] = {0};
rammtx_mutex_t mtx;
size_t next = 0;

static ramfail_status_t fill(char *ptr_arg, size_t sz_arg);
static ramfail_status_t chkfill(char *ptr_arg, size_t sz_arg);
static ramfail_status_t inittest();
static ramfail_status_t testthread(void *arg);
static ramfail_status_t testthread2(int id_arg);
static ramfail_status_t getnext(size_t *next_arg);
static ramfail_status_t dotest();

int main()
{
   RAMFAIL_CONFIRM(-1, RAMFAIL_OK == ramalloc_initialize(NULL, NULL));
   RAMFAIL_CONFIRM(1, RAMFAIL_OK == inittest());
   RAMFAIL_CONFIRM(2, RAMFAIL_OK == dotest());

   return 0;
}

ramfail_status_t inittest()
{
   size_t i = 0, j = 0;

   memset(allocinfos, 0, sizeof(allocinfos));

   RAMFAIL_RETURN(rammtx_mkmutex(&mtx));
   for (i = 0; i < ALLOCATION_COUNT; ++i)
      RAMFAIL_RETURN(rammtx_mkmutex(&allocinfos[i].mtx));
   /* the sequence array must contain two copies of each index into 'allocinfo'.
    * the first will represent an allocation. the second a deallocation. */
   for (i = 0, j = ALLOCATION_COUNT * 2; i < j; ++i)
      sequence[i] = (i / 2);
   /* i shuffle the sequence array to ensure a randomized order of operations. */
   RAMFAIL_RETURN(ramtest_shuffle(sequence, sizeof(sequence[0]), j));
   /* i need to initialize the pools that will be used by the test threads. */
   for (i = 0; i < THREAD_COUNT; ++i)
      RAMFAIL_RETURN(ramlazy_mkpool(&pools[i], RAMOPT_DEFAULTAPPETITE, RECLAIMRATIO));
   return RAMFAIL_OK;
}

static ramfail_status_t dotest()
{
   size_t i = 0;
   ramfail_status_t e0 = RAMFAIL_OK;

   for (i = 0; i < THREAD_COUNT; ++i)
   {
      fprintf(stderr, "[0] starting thread %d...\n", i + 1);
      RAMFAIL_RETURN(ramthread_mkthread(&threads[i], &testthread, (void *)i));
      fprintf(stderr, "[0] started thread %d.\n", i + 1);
   }

   fprintf(stderr, "[0] waiting for threads to finish...\n");
   for (i = 0; i < THREAD_COUNT; ++i)
   {
      ramfail_status_t e = RAMFAIL_INSANE;

      RAMFAIL_RETURN(ramthread_join(&e, threads[i]));
      if (RAMFAIL_OK != e)
      {
         fprintf(stderr,
               "[0] thread %d replied an unexpected failure (%d).\n",
               i + 1, e);
         if (RAMFAIL_OK == e0)
            e0 = e;
      }
   }

   if (RAMFAIL_OK != e0)
      return e0;

   /* now that the threads no longer run, i need to flush them out before
    * quitting. */
   fprintf(stderr, "[0] shutting down.\n");
   for (i = 0; i < THREAD_COUNT; ++i)
      RAMFAIL_RETURN(ramlazy_flush(&pools[i]));

   return RAMFAIL_OK;
}

ramfail_status_t testthread(void *arg)
{
   fprintf(stderr, "[%d] testing...\n", ((int)arg) + 1);
   return testthread2((int)arg);
}

ramfail_status_t getnext(size_t *next_arg)
{
   size_t i = 0;
   size_t j = 0;

   RAMFAIL_DISALLOWZ(next_arg);

   j = ALLOCATION_COUNT * 2;

   RAMFAIL_RETURN(rammtx_wait(&mtx));
   i = next;
   if (next < j)
      ++next;
   RAMFAIL_EPICFAIL(rammtx_quit(&mtx));

   *next_arg = i;
   return RAMFAIL_OK;
}

ramfail_status_t testthread2(int id_arg)
{
   size_t i = 0;
   int allocflag = 1;
   void *nextptr = NULL;
   int32_t nextsz = 0;
   ramlazy_pool_t *nextpool = NULL;
   ramfail_status_t e = RAMFAIL_INSANE;
   ramlazy_pool_t *pool = NULL;

   pool = &pools[id_arg];

   while ((RAMFAIL_OK == (e = getnext(&i))) && i < ALLOCATION_COUNT)
   {
      allocinfo_t *info = NULL;
      void *p = NULL;
      size_t sz = 0;
      ramlazy_pool_t *otherpool = NULL;

      info = &allocinfos[sequence[i]];

      /* i don't want to allocate while i'm holding the allocinfo mutex, so i'll
       * prepare an allocation ahead of time. */
      if (allocflag)
      {
         int32_t roll = 0;

         RAMFAIL_RETURN(ramtest_randuint32(&nextsz, ALLOCATION_MINSIZE, ALLOCATION_MAXSIZE));
         /* i want a certain percentage of allocations to be performed by a different
          * allocator. */
         RAMFAIL_RETURN(ramtest_randuint32(&roll, 0, 100));
         if (roll < MALLOC_PERCENTAGE)
         {
            nextptr = malloc(nextsz);
            nextpool = NULL;
         }
         else
         {
            RAMFAIL_RETURN(ramlazy_acquire(&nextptr, pool, nextsz));
            nextpool = pool;
         }

         RAMFAIL_RETURN(fill(nextptr, nextsz));
      }
      
      RAMFAIL_RETURN(rammtx_wait(&info->mtx));
      /* if there's a pointer stored in 'info->ptr', we'll assume we need to allocate.
       * otherwise, we need to deallocate it. */
      if (NULL == info->ptr)
      {
         info->ptr = nextptr;
         info->sz = nextsz;
         info->pool = nextpool;
         /* i signal to the next loop iteration that i'll need a new allocation. */
         allocflag = 1;
      }
      else
      {
         p = info->ptr;
         info->ptr = NULL;
         sz = info->sz;
         otherpool = info->pool;
      }
      RAMFAIL_EPICFAIL(rammtx_quit(&info->mtx));

      if (p)
      {
         ramlazy_pool_t *querypool = NULL;
         size_t querysz = 0;
         ramfail_status_t f = RAMFAIL_INSANE;

         RAMFAIL_RETURN(chkfill(p, sz));
         f = ramlazy_query(&querypool, &querysz, p);
         switch (f)
         {
         default:
            RAMFAIL_RETURN(f);
            return RAMFAIL_INSANE;
         case RAMFAIL_OK:
            RAMFAIL_CONFIRM(RAMFAIL_CORRUPT, querypool == otherpool);
            /* the size won't be identical due to the nature of muxpools. it won't smaller though. */
            RAMFAIL_CONFIRM(RAMFAIL_CORRUPT, querysz >= sz);
            RAMFAIL_RETURN(ramlazy_release(p));
            break;
         case RAMFAIL_NOTFOUND:
            RAMFAIL_CONFIRM(RAMFAIL_INSANE, NULL == otherpool);
            free(p);
            break;
         }
      }

      RAMFAIL_RETURN(ramlazy_chkpool(pool));
   }

   RAMFAIL_RETURN(ramlazy_flush(pool));
   RAMFAIL_RETURN(ramlazy_chkpool(pool));

   return RAMFAIL_OK;
}

ramfail_status_t fill(char *ptr_arg, size_t sz_arg)
{
   RAMFAIL_DISALLOWZ(ptr_arg);
   RAMFAIL_DISALLOWZ(sz_arg);

   memset(ptr_arg, sz_arg & 0xff, sz_arg);

   return RAMFAIL_OK;
}


ramfail_status_t chkfill(char *ptr_arg, size_t sz_arg)
{
   char *p = NULL, *z = NULL;

   RAMFAIL_DISALLOWZ(ptr_arg);
   RAMFAIL_DISALLOWZ(sz_arg);

   for (p = ptr_arg, z = ptr_arg + sz_arg; 
      p < z && ((char)(sz_arg & 0xff)) == *p; ++p)
      continue;

   if (p != z)
      return RAMFAIL_CORRUPT;

   return RAMFAIL_OK;
}

