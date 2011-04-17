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

#include "shared/test.h"
#include <ramalloc/ramalloc.h>
#include <ramalloc/pg.h>
#include <ramalloc/misc.h>
#include <ramalloc/stdint.h>
#include <stdlib.h>
#include <memory.h>

/* i perform the test with 1k of granular allocations (most likely 64M total). */
#define ALLOCATION_COUNT 1024

static ramfail_status_t fill(char *ptr_arg, size_t index_arg, size_t granularity_arg);
static ramfail_status_t chkfill(char *ptrs_arg[], size_t count_arg, size_t granularity_arg);
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
   char *(p[ALLOCATION_COUNT]) = {0};
   size_t i = 0;
   rampg_pool_t pool;
   size_t granularity = 0;

   RAMFAIL_RETURN(rampg_getgranularity(&granularity));

   RAMFAIL_RETURN(rampg_mkpool(&pool, RAMOPT_DEFAULTAPPETITE));
   RAMFAIL_RETURN(rampg_chkpool(&pool));
   for (i = 0; i < ALLOCATION_COUNT; ++i)
   {
      RAMFAIL_RETURN(rampg_acquire((void **)&p[i], &pool));
      RAMFAIL_RETURN(rampg_chkpool(&pool));
      RAMFAIL_RETURN(fill(p[i], i, granularity));
      RAMFAIL_RETURN(rampg_chkpool(&pool));
      RAMFAIL_RETURN(chkfill(p, ALLOCATION_COUNT, granularity));
   }

   for (i = 0; i < ALLOCATION_COUNT; ++i)
   {
      RAMFAIL_RETURN(rampg_release(p[i]));
      p[i] = NULL;
      RAMFAIL_RETURN(rampg_chkpool(&pool));
      RAMFAIL_RETURN(chkfill(p, ALLOCATION_COUNT, granularity));
   }

   return RAMFAIL_OK;
}

ramfail_status_t randomtest()
{
   char *(p[ALLOCATION_COUNT]) = {0};
   size_t idx[ALLOCATION_COUNT] = {0};
   size_t i = 0;
   rampg_pool_t pool;
   size_t granularity = 0;

   srand(0/*(unsigned int)time(0)*/);
   RAMFAIL_RETURN(rampg_getgranularity(&granularity));

   for (i = 0; i < ALLOCATION_COUNT; ++i)
      idx[i] = i;
   RAMFAIL_RETURN(ramtest_shuffle(idx, sizeof(idx[0]), ALLOCATION_COUNT));

   RAMFAIL_RETURN(rampg_mkpool(&pool, RAMOPT_DEFAULTAPPETITE));
   RAMFAIL_RETURN(rampg_chkpool(&pool));
   for (i = 0; i < ALLOCATION_COUNT; ++i)
   {
      RAMFAIL_RETURN(rampg_acquire((void **)&p[i], &pool));
      RAMFAIL_RETURN(rampg_chkpool(&pool));
      RAMFAIL_RETURN(fill(p[i], i, granularity));
      RAMFAIL_RETURN(rampg_chkpool(&pool));
      RAMFAIL_RETURN(chkfill(p, ALLOCATION_COUNT, granularity));
   }


   for (i = 0; i < ALLOCATION_COUNT; ++i)
   {
      RAMFAIL_RETURN(rampg_release(p[idx[i]]));
      p[idx[i]] = NULL;
      RAMFAIL_RETURN(rampg_chkpool(&pool));
      RAMFAIL_RETURN(chkfill(p, ALLOCATION_COUNT, granularity));
   }

   return RAMFAIL_OK;
}

ramfail_status_t fill(char *ptr_arg, size_t index_arg, size_t granularity_arg)
{
   RAMFAIL_DISALLOWZ(ptr_arg);
   RAMFAIL_DISALLOWZ(granularity_arg);

   memset(ptr_arg, index_arg & 0xff, granularity_arg);

   return RAMFAIL_OK;
}


ramfail_status_t chkfill(char *ptrs_arg[], size_t count_arg, size_t granularity_arg)
{
   size_t i = 0;

   RAMFAIL_DISALLOWZ(ptrs_arg);
   RAMFAIL_DISALLOWZ(count_arg);
   RAMFAIL_DISALLOWZ(granularity_arg);

   for (i = 0; i < count_arg; ++i)
   {
      if (ptrs_arg[i])
      {
         char *p = NULL, *z = NULL;

         for (p = ptrs_arg[i], z = ptrs_arg[i] + granularity_arg; 
            p < z && ((char)(i & 0xff)) == *p; ++p)
            continue;

         if (p != z)
            return RAMFAIL_CORRUPT;
      }
   }

   return RAMFAIL_OK;
}

