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
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#define DEFAULT_ALLOCATION_COUNT 1024 * 100

typedef struct extra
{
   rampg_pool_t e_thepool;
} extra_t;

static ramfail_status_t main2(int argc, char *argv[]);
static ramfail_status_t initdefaults(ramtest_params_t *params_arg);
static ramfail_status_t runtest(const ramtest_params_t *params_arg);
static ramfail_status_t runtest2(const ramtest_params_t *params_arg,
      extra_t *extra_arg);
static ramfail_status_t getpool(rampg_pool_t **pool_arg, void *extra_arg,
      size_t threadidx_arg);
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
   /* the page pool doesn't support parallelized access. */
   params_arg->ramtestp_threadcount = 1;
   /* the page pool doesn't detect detection of foreign pointers. */
   params_arg->ramtestp_mallocchance = 0;
   /* the allocation size has to be determined from the hardware 
    * page size, so i set these to 0 for now. */
   params_arg->ramtestp_minsize = 0;
   params_arg->ramtestp_maxsize = 0;

   return RAMFAIL_OK;
}

ramfail_status_t getpool(rampg_pool_t **pool_arg, void *extra_arg,
      size_t threadidx_arg)
{
   extra_t *x = NULL;

   RAMFAIL_DISALLOWZ(pool_arg);
   *pool_arg = NULL;
   RAMFAIL_DISALLOWZ(extra_arg);
   x = (extra_t *)extra_arg;
   RAMFAIL_CONFIRM(RAMFAIL_RANGE, threadidx_arg >= 0);

   *pool_arg = &x->e_thepool;
   return RAMFAIL_OK;
}

ramfail_status_t acquire(ramtest_allocdesc_t *desc_arg,
      size_t size_arg, void *extra_arg, int threadidx_arg)
{
   rampg_pool_t *pool = NULL;
   void *p = NULL;

   RAMFAIL_DISALLOWZ(desc_arg);
   memset(desc_arg, 0, sizeof(*desc_arg));
   RAMFAIL_DISALLOWZ(size_arg);

   RAMFAIL_RETURN(getpool(&pool, extra_arg, threadidx_arg));
   RAMFAIL_RETURN(rampg_acquire(&p, pool));
   desc_arg->ramtestad_ptr = (char *)p;
   desc_arg->ramtestad_pool = pool;
   desc_arg->ramtestad_sz = size_arg;

   return RAMFAIL_OK;
}

ramfail_status_t release(ramtest_allocdesc_t *desc_arg)
{
   RAMFAIL_DISALLOWZ(desc_arg);

   RAMFAIL_RETURN(rampg_release(desc_arg->ramtestad_ptr));

   return RAMFAIL_OK;
}

ramfail_status_t query(void **pool_arg, size_t *size_arg, void *ptr_arg,
      void *extra_arg)
{
   extra_t *x = NULL;

   RAMFAIL_DISALLOWZ(pool_arg);
   *pool_arg = NULL;
   RAMFAIL_DISALLOWZ(extra_arg);

   x = (extra_t *)extra_arg;
   /* page pools don't support the query option, so i emulate it by
    * returning the pointer stored in the extra info. */
   *pool_arg = &x->e_thepool;
   RAMFAIL_RETURN(rampg_getgranularity(size_arg));
   return RAMFAIL_OK;
}

ramfail_status_t flush(void *extra_arg, int threadidx_arg)
{
   /* algn pools don't support the flush operation. */
   return RAMFAIL_OK;
}

ramfail_status_t check(void *extra_arg, int threadidx_arg)
{
   rampg_pool_t *pool = NULL;

   RAMFAIL_RETURN(getpool(&pool, extra_arg, threadidx_arg));
   RAMFAIL_RETURN(rampg_chkpool(pool));

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
   size_t sz = 0;

   testparams = *params_arg;

   RAMFAIL_RETURN(rampg_getgranularity(&sz));
   if (sz != testparams.ramtestp_maxsize ||
         sz != testparams.ramtestp_minsize)
   {
      fprintf(stderr,
            "warning: this test doesn't support customized sizes. i will "
            "use the predetermined size (%u bytes) for all "
            "allocations.\n", sz);
      testparams.ramtestp_maxsize = sz;
      testparams.ramtestp_minsize = sz;
   }
   /* the pgpool doesn't support multi-threaded access. */
   if (testparams.ramtestp_threadcount > 1)
   {
      fprintf(stderr,
            "the --parallelize option is not supported in this test.\n");
      return RAMFAIL_INPUT;
   }
   /* the pgpool is unable to detect whether it allocated a given
    * object. */
   if (testparams.ramtestp_mallocchance != 0)
   {
      fprintf(stderr,
            "the --mallocchance option is not supported in this test.\n");
      return RAMFAIL_INPUT;
   }

   /* TODO: how do i determine the maximum allocation size ahead of time? */
   testparams.ramtestp_extra = extra_arg;
   testparams.ramtestp_acquire = &acquire;
   testparams.ramtestp_release = &release;
   testparams.ramtestp_query = &query;
   testparams.ramtestp_flush = &flush;
   testparams.ramtestp_check = &check;

   RAMFAIL_RETURN(rampg_mkpool(&extra_arg->e_thepool,
         RAMOPT_DEFAULTAPPETITE));

   RAMFAIL_RETURN(ramtest_test(&testparams));

   return RAMFAIL_OK;
}

