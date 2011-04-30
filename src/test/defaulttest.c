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
#include <ramalloc/default.h>
#include <ramalloc/misc.h>
#include <ramalloc/thread.h>
#include <ramalloc/barrier.h>
#include <ramalloc/stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#define DEFAULT_ALLOCATION_COUNT 1024 * 100
#define DEFAULT_MINIMUM_ALLOCATION_SIZE 8
#define DEFAULT_MAXIMUM_ALLOCATION_SIZE 128
#define DEFAULT_MALLOC_CHANCE 30
/* currently, the reclaim ratio cannot be parameterized. */
#define RECLAIM_RATIO 2

/* currently, i don't need to store extra state to test the default module.
 * i want to keep this test congruent with other tests, so i chose to put
 * a placeholder value inside of it instead of removing the struct. */
typedef struct extra
{
   int e_unused;
} extra_t;

static ramfail_status_t main2(int argc, char *argv[]);
static ramfail_status_t initdefaults(ramtest_params_t *params_arg);
static ramfail_status_t runtest(const ramtest_params_t *params_arg);
static ramfail_status_t runtest2(const ramtest_params_t *params_arg,
      extra_t *extra_arg);
static ramfail_status_t acquire(ramtest_allocdesc_t *desc_arg,
      size_t size_arg, void *extra_arg, int threadidx_arg);
static ramfail_status_t release(ramtest_allocdesc_t *desc_arg);
static ramfail_status_t query(void **pool_arg, size_t *size_arg,
      void *ptr_arg, void *extra_arg);
static ramfail_status_t flush(void *extra_arg, int threadidx_arg);
static ramfail_status_t check(void *extra_arg, int threadidx_arg);

int main(int argc, char *argv[])
{
   ramfail_status_t e = RAMFAIL_INSANE;

   e = main2(argc, argv);
   if (RAMFAIL_OK != e)
      fprintf(stderr, "fail (%d).", e);
   if (RAMFAIL_INPUT == e)
   {
      usage(e, argc, argv);
      ramfail_epicfail("unreachable code.");
      return RAMFAIL_INSANE;
   }
   else
      return e;
}

ramfail_status_t main2(int argc, char *argv[])
{
   ramtest_params_t testparams;
   ramfail_status_t e = RAMFAIL_INSANE;

   RAMFAIL_RETURN(ramalloc_initialize(NULL, NULL));

   RAMFAIL_RETURN(initdefaults(&testparams));
   e = parseargs(&testparams, argc, argv);
   switch (e)
   {
   default:
      RAMFAIL_RETURN(e);
   case RAMFAIL_OK:
      break;
   case RAMFAIL_INPUT:
      return e;
   }

   e = runtest(&testparams);
   switch (e)
   {
   default:
      RAMFAIL_RETURN(e);
   case RAMFAIL_OK:
      break;
   case RAMFAIL_INPUT:
      return e;
   }

   return RAMFAIL_OK;
}

ramfail_status_t initdefaults(ramtest_params_t *params_arg)
{
   RAMFAIL_DISALLOWZ(params_arg);
   memset(params_arg, 0, sizeof(*params_arg));

   params_arg->ramtestp_alloccount = DEFAULT_ALLOCATION_COUNT;
   /* if no thread count is specified, i'll allow the framework to
    * calculate it itself. */
   params_arg->ramtestp_threadcount = 0;
   params_arg->ramtestp_mallocchance = DEFAULT_MALLOC_CHANCE;
   params_arg->ramtestp_minsize = DEFAULT_MINIMUM_ALLOCATION_SIZE;
   params_arg->ramtestp_maxsize = DEFAULT_MAXIMUM_ALLOCATION_SIZE;

   return RAMFAIL_OK;
}

ramfail_status_t acquire(ramtest_allocdesc_t *desc_arg,
      size_t size_arg, void *extra_arg, int threadidx_arg)
{
   void *p = NULL;

   RAMFAIL_DISALLOWZ(desc_arg);
   memset(desc_arg, 0, sizeof(*desc_arg));
   RAMFAIL_DISALLOWZ(size_arg);

   RAMFAIL_RETURN(ramdefault_acquire(&p, size_arg));
   desc_arg->ramtestad_ptr = (char *)p;
   /* the default module doesn't use explicit pool instances. i only need
    * to note whether i'm using the pool or not. i shall use the value of
    * 1 to indicate this. */
   desc_arg->ramtestad_pool = (void *)1;
   desc_arg->ramtestad_sz = size_arg;

   return RAMFAIL_OK;
}

ramfail_status_t release(ramtest_allocdesc_t *desc_arg)
{
   RAMFAIL_DISALLOWZ(desc_arg);

   RAMFAIL_RETURN(ramdefault_discard(desc_arg->ramtestad_ptr));

   return RAMFAIL_OK;
}

ramfail_status_t query(void **pool_arg, size_t *size_arg, void *ptr_arg,
      void *extra_arg)
{
   ramfail_status_t e = RAMFAIL_INSANE;

   RAMFAIL_DISALLOWZ(pool_arg);
   *pool_arg = NULL;
   RAMFAIL_DISALLOWZ(extra_arg);

   e = ramdefault_query(size_arg, ptr_arg);
   switch (e)
   {
   default:
      RAMFAIL_RETURN(e);
      return RAMFAIL_INSANE;
   case RAMFAIL_OK:
      *pool_arg = (void *)1;
      return RAMFAIL_OK;
   case RAMFAIL_NOTFOUND:
      return e;
   }

   return RAMFAIL_OK;
}

ramfail_status_t flush(void *extra_arg, int threadidx_arg)
{
   RAMFAIL_RETURN(ramdefault_flush());

   return RAMFAIL_OK;
}

ramfail_status_t check(void *extra_arg, int threadidx_arg)
{
   RAMFAIL_RETURN(ramdefault_check());

   return RAMFAIL_OK;
}

ramfail_status_t runtest(const ramtest_params_t *params_arg)
{
   ramfail_status_t e = RAMFAIL_INSANE;
   extra_t x = {0};

   RAMFAIL_DISALLOWZ(params_arg);

   e = runtest2(params_arg, &x);

   return e;
}

ramfail_status_t runtest2(const ramtest_params_t *params_arg,
      extra_t *extra_arg)
{
   ramtest_params_t testparams = {0};

   testparams = *params_arg;
   /* i am responsible for policing the minimum and maximum allocation
    * size here. */
   if (testparams.ramtestp_minsize < sizeof(void *) ||
         testparams.ramtestp_maxsize < sizeof(void *))
   {
      fprintf(stderr, "you cannot specify a size smaller than %u bytes.\n",
            sizeof(void *));
      return RAMFAIL_INPUT;
   }
   if (testparams.ramtestp_minsize > testparams.ramtestp_maxsize)
   {
      fprintf(stderr,
            "please specify a minimum size (%u bytes) that is smaller than "
            "or equal to the maximum (%u bytes).\n",
            testparams.ramtestp_minsize, testparams.ramtestp_maxsize);
      return RAMFAIL_INPUT;
   }
   /* TODO: how do i determine the maximum allocation size ahead of time? */
   testparams.ramtestp_extra = extra_arg;
   testparams.ramtestp_acquire = &acquire;
   testparams.ramtestp_release = &release;
   testparams.ramtestp_query = &query;
   testparams.ramtestp_flush = &flush;
   testparams.ramtestp_check = &check;

   RAMFAIL_RETURN(ramtest_test(&testparams));

   return RAMFAIL_OK;
}

