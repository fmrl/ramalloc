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
#include <ramalloc/mux.h>
#include <ramalloc/misc.h>
#include <pstdint.h>
#include <stdlib.h>
#include <memory.h>

#define ALLOCATION_COUNT 1024 * 10
#define ALLOCATION_MINSIZE 4
#define ALLOCATION_MAXSIZE 100

typedef struct allocinfo
{
   char *ptr;
   size_t sz;
} allocinfo_t;

static ramfail_status_t shuffle(void *array_arg, size_t size_arg, size_t count_arg);
static ramfail_status_t randu32ie(uint32_t *result_arg, uint32_t n0_arg, uint32_t n1_arg);
static ramfail_status_t fill(char *ptr_arg, size_t index_arg, size_t granularity_arg);
static ramfail_status_t chkfill(allocinfo_t ptrs_arg[], size_t count_arg);
static ramfail_status_t sequentialtest();
static ramfail_status_t randomtest();

int main()
{
   RAMFAIL_CONFIRM(-1, RAMFAIL_OK == ramalloc_initialize(NULL, NULL));
   RAMFAIL_CONFIRM(1, RAMFAIL_OK == sequentialtest());
   RAMFAIL_CONFIRM(2, RAMFAIL_OK == randomtest());

   return 0;
}

ramfail_status_t sequentialtest()
{
   allocinfo_t ai[ALLOCATION_COUNT] = {0};
   size_t i = 0;
   rammux_pool_t pool;

   RAMFAIL_RETURN(rammux_mkpool(&pool, RAMOPT_DEFAULTAPPETITE));
   RAMFAIL_RETURN(rammux_chkpool(&pool));
   for (i = 0; i < ALLOCATION_COUNT; ++i)
   {
      uint32_t sz = 0;

      RAMFAIL_RETURN(randu32ie(&sz, ALLOCATION_MINSIZE, ALLOCATION_MAXSIZE));
      RAMFAIL_RETURN(rammux_acquire(&ai[i].ptr, &pool, sz));
      ai[i].sz = sz;
      RAMFAIL_RETURN(rammux_chkpool(&pool));
      RAMFAIL_RETURN(fill(ai[i].ptr, i, sz));
      RAMFAIL_RETURN(rammux_chkpool(&pool));
      RAMFAIL_RETURN(chkfill(ai, ALLOCATION_COUNT));
   }

   for (i = 0; i < ALLOCATION_COUNT; ++i)
   {
      RAMFAIL_RETURN(rammux_release(ai[i].ptr));
      ai[i].ptr = NULL;
      ai[i].sz = 0;
      RAMFAIL_RETURN(rammux_chkpool(&pool));
      RAMFAIL_RETURN(chkfill(ai, ALLOCATION_COUNT));
   }

   return RAMFAIL_OK;
}

ramfail_status_t randomtest()
{
   allocinfo_t ai[ALLOCATION_COUNT] = {0};
   size_t idx[ALLOCATION_COUNT] = {0};
   size_t i = 0;
   rammux_pool_t pool;

   srand(0/*(unsigned int)time(0)*/);

   for (i = 0; i < ALLOCATION_COUNT; ++i)
      idx[i] = i;
   RAMFAIL_RETURN(shuffle(idx, sizeof(idx[0]), ALLOCATION_COUNT));

   RAMFAIL_RETURN(rammux_mkpool(&pool, RAMOPT_DEFAULTAPPETITE));
   RAMFAIL_RETURN(rammux_chkpool(&pool));
   for (i = 0; i < ALLOCATION_COUNT; ++i)
   {
      uint32_t sz = 0;

      RAMFAIL_RETURN(randu32ie(&sz, ALLOCATION_MINSIZE, ALLOCATION_MAXSIZE));
      RAMFAIL_RETURN(rammux_acquire(&ai[i].ptr, &pool, sz));
      ai[i].sz = sz;
      RAMFAIL_RETURN(rammux_chkpool(&pool));
      RAMFAIL_RETURN(fill(ai[i].ptr, i, sz));
      RAMFAIL_RETURN(rammux_chkpool(&pool));
      RAMFAIL_RETURN(chkfill(ai, ALLOCATION_COUNT));
   }

   for (i = 0; i < ALLOCATION_COUNT; ++i)
   {
      RAMFAIL_RETURN(rammux_release(ai[idx[i]].ptr));
      ai[idx[i]].ptr = NULL;
      ai[idx[i]].sz = 0;
      RAMFAIL_RETURN(rammux_chkpool(&pool));
      RAMFAIL_RETURN(chkfill(ai, ALLOCATION_COUNT));
   }

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

ramfail_status_t fill(char *ptr_arg, size_t index_arg, size_t granularity_arg)
{
   RAMFAIL_DISALLOWZ(ptr_arg);
   RAMFAIL_DISALLOWZ(granularity_arg);

   memset(ptr_arg, index_arg & 0xff, granularity_arg);

   return RAMFAIL_OK;
}


ramfail_status_t chkfill(allocinfo_t ptrs_arg[], size_t count_arg)
{
   size_t i = 0;

   RAMFAIL_DISALLOWZ(ptrs_arg);
   RAMFAIL_DISALLOWZ(count_arg);

   for (i = 0; i < count_arg; ++i)
   {
      if (ptrs_arg[i].ptr)
      {
         char *p = NULL, *z = NULL;

         for (p = ptrs_arg[i].ptr, z = ptrs_arg[i].ptr + ptrs_arg[i].sz; 
            p < z && ((char)(i & 0xff)) == *p; ++p)
            continue;

         if (p != z)
            return RAMFAIL_CORRUPT;
      }
   }

   return RAMFAIL_OK;
}

