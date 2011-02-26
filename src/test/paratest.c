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
#include <ramalloc/para.h>
#include <ramalloc/misc.h>
#include <ramalloc/thread.h>
#include <ramalloc/barrier.h>
#include <pstdint.h>
#include <stdlib.h>
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
   rampara_pool_t *pool;
   rammtx_mutex_t mtx;
} allocinfo_t;

allocinfo_t allocinfos[ALLOCATION_COUNT] = {0};
size_t sequence[ALLOCATION_COUNT * 2] = {0};
rampara_pool_t thepool;
rammtx_mutex_t mtx;
size_t next = 0;
int ok = 1;
rambarrier_barrier_t finish;

static ramfail_status_t shuffle(void *array_arg, size_t size_arg, size_t count_arg);
static ramfail_status_t randu32ie(uint32_t *result_arg, uint32_t n0_arg, uint32_t n1_arg);
static ramfail_status_t fill(char *ptr_arg, size_t sz_arg);
static ramfail_status_t chkfill(char *ptr_arg, size_t sz_arg);
static ramfail_status_t inittest();
static int32_t testthread(void *arg);
static ramfail_status_t testthread2(rampara_pool_t *pool_arg);
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
   RAMFAIL_RETURN(shuffle(sequence, sizeof(sequence[0]), j));
   /* i need to initialize the pool. */
   RAMFAIL_RETURN(rampara_mkpool(&thepool, RAMOPT_DEFAULTAPPETITE, RECLAIMRATIO));
   /* the barrier needs to include the test threads and the main thread. */
   RAMFAIL_RETURN(rambarrier_mkbarrier(&finish, THREAD_COUNT + 1));
   return RAMFAIL_OK;
}

static ramfail_status_t dotest()
{
   size_t i = 0;

   for (i = 0; i < THREAD_COUNT; ++i)
      RAMFAIL_RETURN(ramthread_mkthread(&testthread, &thepool));

   RAMFAIL_RETURN(rambarrier_wait(&finish));

   if (ok)
   {
      /* now that the threads no longer run, i need to flush them out before quitting.
       * this doesn't work all that well yet, however, and some memory will be leaked. */
      for (i = 0; i < THREAD_COUNT; ++i)
         RAMFAIL_RETURN(rampara_flush(&thepool));
      return RAMFAIL_OK;
   }
   else
      return RAMFAIL_CORRUPT;
}

int32_t testthread(void *arg)
{
   ramfail_status_t e = RAMFAIL_INSANE;
   rampara_pool_t *pool = NULL;
   
   pool = (rampara_pool_t *)arg;
   e = testthread2(pool);
   if (RAMFAIL_OK != e)
      ok = 0;

   RAMFAIL_EPICFAIL(rambarrier_wait(&finish));

   return (int32_t)e;
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

ramfail_status_t testthread2(rampara_pool_t *pool_arg)
{
   size_t i = 0;
   int allocflag = 1;
   void *nextptr = NULL;
   int32_t nextsz = 0;
   rampara_pool_t *nextpool = NULL;
   ramfail_status_t e = RAMFAIL_INSANE;

   while ((RAMFAIL_OK == (e = getnext(&i))) && i < ALLOCATION_COUNT)
   {
      allocinfo_t *info = NULL;
      void *p = NULL;
      size_t sz = 0;
      rampara_pool_t *otherpool = NULL;

      info = &allocinfos[sequence[i]];

      /* i don't want to allocate while i'm holding the allocinfo mutex, so i'll
       * prepare an allocation ahead of time. */
      if (allocflag)
      {
         int32_t roll = 0;

         RAMFAIL_RETURN(randu32ie(&nextsz, ALLOCATION_MINSIZE, ALLOCATION_MAXSIZE));
         /* i want a certain percentage of allocations to be performed by a different
          * allocator. */
         RAMFAIL_RETURN(randu32ie(&roll, 0, 100));
         if (roll < MALLOC_PERCENTAGE)
         {
            nextptr = malloc(nextsz);
            nextpool = NULL;
         }
         else
         {
            RAMFAIL_RETURN(rampara_acquire(&nextptr, pool_arg, nextsz));
            nextpool = pool_arg;
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
         rampara_pool_t *querypool = NULL;
         size_t querysz = 0;
         ramfail_status_t f = RAMFAIL_INSANE;

         RAMFAIL_RETURN(chkfill(p, sz));
         f = rampara_query(&querypool, &querysz, p);
         switch (f)
         {
         default:
            RAMFAIL_RETURN(f);
            return RAMFAIL_INSANE;
         case RAMFAIL_OK:
            RAMFAIL_CONFIRM(RAMFAIL_CORRUPT, querypool == otherpool);
            /* the size won't be identical due to the nature of muxpools. it won't smaller though. */
            RAMFAIL_CONFIRM(RAMFAIL_CORRUPT, querysz >= sz);
            RAMFAIL_RETURN(rampara_release(p));
            break;
         case RAMFAIL_NOTFOUND:
            RAMFAIL_CONFIRM(RAMFAIL_INSANE, NULL == otherpool);
            free(p);
            break;
         }
      }

      RAMFAIL_RETURN(rampara_chkpool(pool_arg));
   }

   RAMFAIL_RETURN(rampara_flush(pool_arg));
   RAMFAIL_RETURN(rampara_chkpool(pool_arg));

   return RAMFAIL_OK;
}

ramfail_status_t shuffle(void *array_arg, size_t size_arg, size_t count_arg)
{
   char *p = (char *)array_arg;
   size_t i = 0;

   RAMFAIL_DISALLOWZ(array_arg);
   RAMFAIL_DISALLOWZ(size_arg);

   if (0 < count_arg)
   {
      for (i = count_arg - 1; i > 0; --i)
      {
         uint32_t j = 0;

         RAMFAIL_RETURN(randu32ie(&j, 0, i));
         RAMFAIL_RETURN(rammisc_swap(&p[i * size_arg], &p[j * size_arg], size_arg));
      }
   }

   return RAMFAIL_OK;
}

ramfail_status_t randu32ie(uint32_t *result_arg, uint32_t n0_arg, uint32_t n1_arg)
{
   uint32_t n = 0;

   RAMFAIL_DISALLOWZ(result_arg);
   RAMFAIL_CONFIRM(RAMFAIL_DISALLOWED, n0_arg <= n1_arg);

#define RANDU32IE(R, N0, N1) (((uint32_t)((float)((N1) - (N0)) * (R) / ((float)RAND_MAX + 1))) + (N0))
   /* this assertion tests the boundaries of the scaling formula. */
   assert(RANDU32IE(0, n0_arg, n1_arg) >= n0_arg);
   assert(RANDU32IE(RAND_MAX, n0_arg, n1_arg) < n1_arg);
   n = RANDU32IE(rand(), n0_arg, n1_arg);
#undef RANDU32IE

   assert(n >= n0_arg);
   assert(n < n1_arg);
   *result_arg = n;
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

