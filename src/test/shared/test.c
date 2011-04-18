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

#include "test.h"
#include <ramalloc/mtx.h>
#include <ramalloc/thread.h>
#include <ramalloc/misc.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

/* TODO: calls to fprinf() should be checked. */

typedef struct ramtest_allocrec
{
   ramtest_allocdesc_t ramtestar_desc;
   rammtx_mutex_t ramtestar_mtx;
} ramtest_allocrec_t;

typedef struct ramtest_test
{
   ramtest_params_t ramtestt_params;
   ramtest_allocrec_t *ramtestt_records;
   ramthread_thread_t *ramtestt_threads;
   rammtx_mutex_t ramtestt_mtx;
   size_t ramtestt_nextrec;
   size_t *ramtestt_sequence;
} ramtest_test_t;

typedef struct ramtest_start
{
   ramtest_test_t *ramtests_test;
   int ramtests_threadidx;
} ramtest_start_t;

static ramfail_status_t ramtest_inittest(ramtest_test_t *test_arg,
      const ramtest_params_t *params_arg);
static ramfail_status_t ramtest_fintest(ramtest_test_t *test_arg);
static ramfail_status_t ramtest_inittest2(ramtest_test_t *test_arg,
      const ramtest_params_t *params_arg);
static ramfail_status_t ramtest_describe(FILE *out_arg,
      const ramtest_params_t *params_arg);
static ramfail_status_t ramtest_test2(ramtest_test_t *test_arg);
static ramfail_status_t ramtest_start(ramtest_test_t *test_arg);
static ramfail_status_t ramtest_thread(void *arg);
static ramfail_status_t ramtest_thread2(ramtest_test_t *test_arg,
      int threadidx_arg);
static ramfail_status_t ramtest_next(size_t *next_arg,
      ramtest_test_t *test_arg);
static ramfail_status_t ramtest_join(ramtest_test_t *test_arg);
static ramfail_status_t ramtest_alloc(ramtest_allocdesc_t *desc_arg,
      ramtest_test_t *test_arg, int threadidx_arg);
static ramfail_status_t ramtest_dealloc(ramtest_allocdesc_t *ptrdesc_arg,
      ramtest_test_t *test_arg, int threadidx_arg);
static ramfail_status_t ramtest_fill(char *ptr_arg, size_t sz_arg);
static ramfail_status_t ramtest_chkfill(char *ptr_arg, size_t sz_arg);

#define RAMTEST_RANDUINT32(R, N0, N1) \
         (((uint32_t)((double)((N1) - (N0)) * (R) / \
         ((double)RAND_MAX + 1))) + (N0))

ramfail_status_t ramtest_randuint32(uint32_t *result_arg, uint32_t n0_arg,
      uint32_t n1_arg)
{
   uint32_t n = 0;

   RAMFAIL_DISALLOWZ(result_arg);
   RAMFAIL_CONFIRM(RAMFAIL_DISALLOWED, n0_arg <= n1_arg);

   /* this assertion tests the boundaries of the scaling formula. */
   assert(RAMTEST_RANDUINT32(0, n0_arg, n1_arg) >= n0_arg);
   assert(RAMTEST_RANDUINT32(RAND_MAX, n0_arg, n1_arg) < n1_arg);
   n = RAMTEST_RANDUINT32(rand(), n0_arg, n1_arg);
#undef RAMTEST_RANDUINT32

   assert(n >= n0_arg);
   assert(n < n1_arg);
   *result_arg = n;
   return RAMFAIL_OK;
}

ramfail_status_t ramtest_shuffle(void *array_arg, size_t size_arg,
      size_t count_arg)
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

         RAMFAIL_RETURN(ramtest_randuint32(&j, 0, i));
         RAMFAIL_RETURN(rammisc_swap(&p[i * size_arg], &p[j * size_arg],
               size_arg));
      }
   }

   return RAMFAIL_OK;
}

ramfail_status_t ramtest_inittest(ramtest_test_t *test_arg,
      const ramtest_params_t *params_arg)
{
   ramfail_status_t e = RAMFAIL_INSANE;

   RAMFAIL_DISALLOWZ(test_arg);

   memset(test_arg, 0, sizeof(*test_arg));
   e = ramtest_inittest2(test_arg, params_arg);
   if (RAMFAIL_OK == e)
      return RAMFAIL_OK;
   {
      RAMFAIL_EPICFAIL(ramtest_fintest(test_arg));
      RAMFAIL_RETURN(e);
      return RAMFAIL_INSANE;
   }
}

ramfail_status_t ramtest_inittest2(ramtest_test_t *test_arg,
      const ramtest_params_t *params_arg)
{
   size_t i = 0;
   size_t seqlen = 0;

   RAMFAIL_DISALLOWZ(params_arg);
   RAMFAIL_DISALLOWZ(params_arg->ramtestp_alloccount);
   RAMFAIL_DISALLOWZ(params_arg->ramtestp_threadcount);
   RAMFAIL_CONFIRM(RAMFAIL_RANGE, params_arg->ramtestp_minsize > 0);
   RAMFAIL_CONFIRM(RAMFAIL_RANGE,
         params_arg->ramtestp_minsize <= params_arg->ramtestp_maxsize);
   RAMFAIL_CONFIRM(RAMFAIL_RANGE, params_arg->ramtestp_mallocchance >= 0);
   RAMFAIL_CONFIRM(RAMFAIL_RANGE, params_arg->ramtestp_mallocchance <= 100);
   RAMFAIL_DISALLOWZ(params_arg->ramtestp_acquire);
   RAMFAIL_DISALLOWZ(params_arg->ramtestp_release);
   RAMFAIL_DISALLOWZ(params_arg->ramtestp_query);
   /* *params_arg->ramtestp_flush* is allowed to be NULL. */
   RAMFAIL_DISALLOWZ(params_arg->ramtestp_check);

   test_arg->ramtestt_params = *params_arg;
   test_arg->ramtestt_records =
         calloc(test_arg->ramtestt_params.ramtestp_alloccount,
         sizeof(*test_arg->ramtestt_records));
   RAMFAIL_CONFIRM(RAMFAIL_RESOURCE, NULL != test_arg->ramtestt_records);
   test_arg->ramtestt_threads =
         calloc(test_arg->ramtestt_params.ramtestp_threadcount,
         sizeof(*test_arg->ramtestt_threads));
   RAMFAIL_CONFIRM(RAMFAIL_RESOURCE, NULL != test_arg->ramtestt_threads);
   seqlen = test_arg->ramtestt_params.ramtestp_alloccount * 2;
   test_arg->ramtestt_sequence = calloc(seqlen,
         sizeof(*test_arg->ramtestt_sequence));
   RAMFAIL_CONFIRM(RAMFAIL_RESOURCE, NULL != test_arg->ramtestt_sequence);

   RAMFAIL_RETURN(rammtx_mkmutex(&test_arg->ramtestt_mtx));
   for (i = 0; i < test_arg->ramtestt_params.ramtestp_alloccount; ++i)
   {
      RAMFAIL_RETURN(rammtx_mkmutex(
            &test_arg->ramtestt_records[i].ramtestar_mtx));
   }
   /* the sequence array must contain two copies of each index into
    * *test_arg->ramtestt_records*. the first represents an allocation.
    * the second, a deallocation. */
   for (i = 0; i < seqlen; ++i)
      test_arg->ramtestt_sequence[i] = (i / 2);
   /* i shuffle the sequence array to ensure a randomized order of
    * operations. */
   RAMFAIL_RETURN(ramtest_shuffle(test_arg->ramtestt_sequence,
         sizeof(test_arg->ramtestt_sequence[0]), seqlen));

   if (!test_arg->ramtestt_params.ramtestp_userngseed)
      test_arg->ramtestt_params.ramtestp_rngseed = (unsigned int)time(NULL);
   srand(test_arg->ramtestt_params.ramtestp_rngseed);
   fprintf(stderr, "[0] i seeded the random generator with the value %u.\n",
         test_arg->ramtestt_params.ramtestp_rngseed);

   test_arg->ramtestt_nextrec = 0;

   return RAMFAIL_OK;
}

ramfail_status_t ramtest_fintest(ramtest_test_t *test_arg)
{
   RAMFAIL_DISALLOWZ(test_arg);

   if (NULL != test_arg->ramtestt_records)
      free(test_arg->ramtestt_records);
   if (NULL != test_arg->ramtestt_threads)
   {
      /* TODO: tear down thread structures. */
      free(test_arg->ramtestt_threads);
   }
   if (NULL != test_arg->ramtestt_sequence)
      free(test_arg->ramtestt_sequence);

   return RAMFAIL_OK;
}

ramfail_status_t ramtest_describe(FILE *out_arg,
      const ramtest_params_t *params_arg)
{
   RAMFAIL_DISALLOWZ(out_arg);
   RAMFAIL_DISALLOWZ(params_arg);

   if (params_arg->ramtestp_dryrun)
      fprintf(out_arg, "you have specified the following test:\n\n");
   else
      fprintf(out_arg, "i will run the following test:\n\n");
   fprintf(out_arg, "%u allocation(s) (and corresponding "
         "deallocations).\n",
         params_arg->ramtestp_alloccount);
   fprintf(out_arg, "%u parallel operation(s) allowed.\n",
         params_arg->ramtestp_threadcount);
   fprintf(out_arg, "%d%% of the allocations will be managed by malloc() "
         "and free().\n", params_arg->ramtestp_mallocchance);
   fprintf(out_arg, "allocations will not be smaller than %u bytes.\n",
         params_arg->ramtestp_minsize);
   fprintf(out_arg, "allocations will not be larger than %u bytes.\n",
         params_arg->ramtestp_maxsize);
   if (params_arg->ramtestp_userngseed)
   {
      fprintf(out_arg, "the random number generator will use seed %u.\n",
            params_arg->ramtestp_rngseed);
   }
   else
   {
      fprintf(out_arg, "the random number generator will use a randomly "
            "selected seed.\n");
   }
   if (params_arg->ramtestp_dryrun)
      fprintf(out_arg, "\nto run this test, omit the --dry-run option.");
   else
      fprintf(out_arg, "-----\n");

   return RAMFAIL_OK;
}

ramfail_status_t ramtest_test(const ramtest_params_t *params_arg)
{
   ramfail_status_t e = RAMFAIL_INSANE;
   ramtest_test_t test = {0};

   RAMFAIL_DISALLOWZ(params_arg);

   RAMFAIL_RETURN(ramtest_describe(stderr, params_arg));

   /* if a dry run has been specified, i'll quit now. */
   if (params_arg->ramtestp_dryrun)
      return RAMFAIL_OK;

   RAMFAIL_RETURN(ramtest_inittest(&test, params_arg));

   e = ramtest_test2(&test);
   RAMFAIL_RETURN(ramfail_accumulate(&e, ramtest_fintest(&test)));

   if (RAMFAIL_OK == e)
   {
      fprintf(stderr, "[0] the test succeeded.\n");
      return RAMFAIL_OK;
   }
   else
   {
      fprintf(stderr, "[0] the test failed (reply code %d).\n", e);
      RAMFAIL_RETURN(e);
      return RAMFAIL_INSANE;
   }
}

ramfail_status_t ramtest_test2(ramtest_test_t *test_arg)
{
   fprintf(stderr, "[0] beginning test...\n");

   RAMFAIL_RETURN(ramtest_start(test_arg));
   RAMFAIL_RETURN(ramtest_join(test_arg));

   return RAMFAIL_OK;
}

ramfail_status_t ramtest_start(ramtest_test_t *test_arg)
{
   int i = 0;

   RAMFAIL_DISALLOWZ(test_arg);

   for (i = 0; i < test_arg->ramtestt_params.ramtestp_threadcount; ++i)
   {
      ramtest_start_t *start = NULL;

      fprintf(stderr, "[0] starting thread %d...\n", i + 1);
      /* i'm the sole producer of this memory; *ramtest_start()* is the sole
       * consumer. */
      start = (ramtest_start_t *)calloc(sizeof(*start), 1);
      RAMFAIL_CONFIRM(RAMFAIL_RESOURCE, NULL != start);
      start->ramtests_test = test_arg;
      start->ramtests_threadidx = i;
      RAMFAIL_RETURN(ramthread_mkthread(&test_arg->ramtestt_threads[i],
            &ramtest_thread, start));
      fprintf(stderr, "[0] started thread %d.\n", i + 1);
   }

   return RAMFAIL_OK;
}

ramfail_status_t ramtest_join(ramtest_test_t *test_arg)
{
   int i = 0;
   ramfail_status_t myreply = RAMFAIL_OK;

   RAMFAIL_DISALLOWZ(test_arg);

   fprintf(stderr, "[0] i am waiting for my threads to finish...\n");
   for (i = 0; i < test_arg->ramtestt_params.ramtestp_threadcount; ++i)
   {
      ramfail_status_t e = RAMFAIL_INSANE;

      RAMFAIL_RETURN(ramthread_join(&e, test_arg->ramtestt_threads[i]));
      if (RAMFAIL_OK != e)
      {
         fprintf(stderr,
               "[0] thread %d replied with an unexpected failure (%d).\n",
               i + 1, e);
         /* if i haven't yet recorded an error as my reply, do so now. this
          * ensures that the primary symptom is recorded and not any echoes
          * of the problem. */
         RAMFAIL_RETURN(ramfail_accumulate(&myreply, e));
      }
   }

   return myreply;
}


ramfail_status_t ramtest_next(size_t *next_arg, ramtest_test_t *test_arg)
{
   size_t i = 0;
   size_t j = 0;

   RAMFAIL_DISALLOWZ(next_arg);
   RAMFAIL_DISALLOWZ(test_arg);

   j = test_arg->ramtestt_params.ramtestp_alloccount;

   RAMFAIL_RETURN(rammtx_wait(&test_arg->ramtestt_mtx));
   i = test_arg->ramtestt_nextrec;
   if (i < j)
      ++test_arg->ramtestt_nextrec;
   RAMFAIL_EPICFAIL(rammtx_quit(&test_arg->ramtestt_mtx));

   *next_arg = i;
   return RAMFAIL_OK;
}

ramfail_status_t ramtest_thread(void *arg)
{
   ramtest_start_t *start = (ramtest_start_t *)arg;
   ramtest_test_t *test = NULL;
   int threadidx = -1;
   ramfail_status_t e = RAMFAIL_INSANE;

   RAMFAIL_DISALLOWZ(arg);

   test = start->ramtests_test;
   threadidx = start->ramtests_threadidx;
   /* i'm the sole consumer of this memory; *ramtest_start()* is the sole
    * producer. */
   free(start);
   fprintf(stderr, "[%d] testing...\n", threadidx + 1);
   e = ramtest_thread2(test, threadidx);
   fprintf(stderr, "[%d] finished.\n", threadidx + 1);

   return e;
}

ramfail_status_t ramtest_thread2(ramtest_test_t *test_arg,
      int threadidx_arg)
{
   size_t i = 0;
   ramfail_status_t e = RAMFAIL_INSANE;
   int cachedflag = 0;
   ramtest_allocdesc_t cached = {0};

   RAMFAIL_DISALLOWZ(test_arg);
   RAMFAIL_CONFIRM(RAMFAIL_RANGE, threadidx_arg >= 0);
   RAMFAIL_CONFIRM(RAMFAIL_RANGE,
         threadidx_arg < test_arg->ramtestt_params.ramtestp_threadcount);

   while ((RAMFAIL_OK == (e = ramtest_next(&i, test_arg)))
         && i < test_arg->ramtestt_params.ramtestp_alloccount)
   {
      ramtest_allocrec_t *info = NULL;
      ramtest_allocdesc_t condemned = {0};

      info = &test_arg->ramtestt_records[test_arg->ramtestt_sequence[i]];
      /* i don't want to allocate while i'm holding the allocation record
       * mutex, so i'll prepare an allocation ahead of time. */
      if (!cachedflag)
      {
         RAMFAIL_RETURN(ramtest_alloc(&cached, test_arg,
               threadidx_arg));
      }
      /* there's actually a race condition between the call to
       * *ramtest_next()* and this point. the worst that could happen
       * (i think) is  that the first thread to draw a given record's index
       * might end up being the deallocating thread. */
      RAMFAIL_RETURN(rammtx_wait(&info->ramtestar_mtx));
      /* if there's a pointer stored in *info->ramtestar_desc.ramtestad_ptr*
       * we'll assume we're the allocating thread. otherwise, we need to
       * deallocate. */
      if (NULL == info->ramtestar_desc.ramtestad_ptr)
      {
         info->ramtestar_desc = cached;
         /* i signal to the next loop iteration that i'll need a new
          * allocation. */
         cachedflag = 0;
      }
      else
         condemned = info->ramtestar_desc;
      RAMFAIL_EPICFAIL(rammtx_quit(&info->ramtestar_mtx));
      /* if i have a condemned pointer, i need to deallocate it. */
      if (condemned.ramtestad_ptr != NULL)
      {
         RAMFAIL_RETURN(ramtest_dealloc(&condemned, test_arg,
               threadidx_arg));
         condemned.ramtestad_ptr = NULL;
      }

      RAMFAIL_RETURN(test_arg->ramtestt_params.ramtestp_check(
            test_arg->ramtestt_params.ramtestp_extra, threadidx_arg));
   }

   RAMFAIL_RETURN(test_arg->ramtestt_params.ramtestp_flush(
         test_arg->ramtestt_params.ramtestp_extra, threadidx_arg));
   RAMFAIL_RETURN(test_arg->ramtestt_params.ramtestp_check(
         test_arg->ramtestt_params.ramtestp_extra, threadidx_arg));

   return RAMFAIL_OK;
}

ramfail_status_t ramtest_alloc(ramtest_allocdesc_t *newptr_arg,
      ramtest_test_t *test_arg, int threadidx_arg)
{
   uint32_t roll = 0;
   ramtest_allocdesc_t desc = {0};

   RAMFAIL_DISALLOWZ(newptr_arg);
   memset(newptr_arg, 0, sizeof(*newptr_arg));
   RAMFAIL_DISALLOWZ(test_arg);
   RAMFAIL_CONFIRM(RAMFAIL_RANGE, threadidx_arg >= 0);
   RAMFAIL_CONFIRM(RAMFAIL_RANGE,
         threadidx_arg < test_arg->ramtestt_params.ramtestp_threadcount);

   RAMFAIL_RETURN(ramtest_randuint32(&desc.ramtestad_sz,
         test_arg->ramtestt_params.ramtestp_minsize,
         test_arg->ramtestt_params.ramtestp_maxsize));
   /* i want a certain percentage of allocations to be performed by
    * an alternate allocator. */
   RAMFAIL_RETURN(ramtest_randuint32(&roll, 0, 100));
   if (roll < test_arg->ramtestt_params.ramtestp_mallocchance)
   {
      desc.ramtestad_pool = NULL;
      desc.ramtestad_ptr = malloc(desc.ramtestad_sz);
   }
   else
   {
      RAMFAIL_RETURN(test_arg->ramtestt_params.ramtestp_acquire(&desc,
            desc.ramtestad_sz, test_arg->ramtestt_params.ramtestp_extra,
            threadidx_arg));
   }

   RAMFAIL_RETURN(ramtest_fill(desc.ramtestad_ptr, desc.ramtestad_sz));

   *newptr_arg = desc;
   return RAMFAIL_OK;
}

ramfail_status_t ramtest_dealloc(ramtest_allocdesc_t *ptrdesc_arg,
      ramtest_test_t *test_arg, int threadidx_arg)
{
   void *pool = NULL;
   size_t sz = 0;
   ramfail_status_t e = RAMFAIL_INSANE;

   RAMFAIL_DISALLOWZ(ptrdesc_arg);
   RAMFAIL_DISALLOWZ(ptrdesc_arg->ramtestad_ptr);
   RAMFAIL_DISALLOWZ(test_arg);
   RAMFAIL_CONFIRM(RAMFAIL_RANGE, threadidx_arg >= 0);
   RAMFAIL_CONFIRM(RAMFAIL_RANGE,
         threadidx_arg < test_arg->ramtestt_params.ramtestp_threadcount);

   RAMFAIL_RETURN(ramtest_chkfill(ptrdesc_arg->ramtestad_ptr,
         ptrdesc_arg->ramtestad_sz));
   e = test_arg->ramtestt_params.ramtestp_query(&pool, &sz,
         ptrdesc_arg->ramtestad_ptr);
   switch (e)
   {
   default:
      RAMFAIL_RETURN(e);
      return RAMFAIL_INSANE;
   case RAMFAIL_OK:
      RAMFAIL_CONFIRM(RAMFAIL_CORRUPT,
            ptrdesc_arg->ramtestad_pool == pool);
      /* the size won't always be identical due to the nature of mux pools.
       * the size will never be smaller, though. */
      RAMFAIL_CONFIRM(RAMFAIL_CORRUPT, sz >= ptrdesc_arg->ramtestad_sz);
      RAMFAIL_RETURN(
            test_arg->ramtestt_params.ramtestp_release(ptrdesc_arg));
      break;
   case RAMFAIL_NOTFOUND:
      RAMFAIL_CONFIRM(RAMFAIL_INSANE, NULL == ptrdesc_arg->ramtestad_pool);
      free(ptrdesc_arg->ramtestad_ptr);
      break;
   }

   return RAMFAIL_OK;
}

ramfail_status_t ramtest_fill(char *ptr_arg, size_t sz_arg)
{
   RAMFAIL_DISALLOWZ(ptr_arg);
   RAMFAIL_DISALLOWZ(sz_arg);

   memset(ptr_arg, sz_arg & 0xff, sz_arg);

   return RAMFAIL_OK;
}

ramfail_status_t ramtest_chkfill(char *ptr_arg, size_t sz_arg)
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
