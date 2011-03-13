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

/* this file should not compile anything if the appropriate platform
 * preprocessor definition isn't available ('RAMSYS_UNIX' in this
 * case). */
#include <ramalloc/sys.h>
#ifdef RAMSYS_UNIX

#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>


typedef struct ramuix_startroutine
{
   ramsys_threadmain_t ramuixsr_main;
   void *ramuixsr_arg;
   ramuix_barrier_t ramuixsr_barrier;
} ramuix_startroutine_t;

static void *ramuix_startroutine(void *arg);
static ramfail_status_t
   ramlin_waitonbarrier2(ramuix_barrier_t *barrier_arg);

static const ramsys_globals_t *ramuix_theglobals;

ramfail_status_t ramuix_mkglobals(ramsys_globals_t *globals_arg)
{
   assert(!globals_arg->ramsysg_initflag);

   errno = 0;
   globals_arg->ramsysg_granularity = sysconf(_SC_PAGESIZE);
   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM, 0 == errno);
   errno = 0;
   globals_arg->ramsysg_pagesize = sysconf(_SC_PAGESIZE);
   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM, 0 == errno);

   /* i'm allowed to cache 'globals_arg' since it's a singleton. */
   ramuix_theglobals = globals_arg;

   return RAMFAIL_OK;
}

ramfail_status_t ramuix_commit(char *page_arg)
{
   /* there's no difference between commit and reserve on linux. */
   assert(ramuix_theglobals != NULL);
   assert(ramuix_theglobals->ramsysg_granularity ==
         ramuix_theglobals->ramsysg_pagesize);
   return RAMFAIL_OK;
}

ramfail_status_t ramuix_decommit(char *page_arg)
{
   /* there's no difference between commit and reserve on linux. */
   assert(ramuix_theglobals != NULL);
   assert(ramuix_theglobals->ramsysg_granularity ==
         ramuix_theglobals->ramsysg_pagesize);
   return RAMFAIL_OK;
}

ramfail_status_t ramuix_reset(char *page_arg)
{
   return ramuix_decommit(page_arg);
}

ramfail_status_t ramuix_reserve(char **pages_arg)
{
   char *p = NULL;

   RAMFAIL_DISALLOWZ(pages_arg);
   *pages_arg = NULL;
   RAMFAIL_CONFIRM(RAMFAIL_UNINITIALIZED, ramuix_theglobals != NULL);

   p = mmap(NULL, ramuix_theglobals->ramsysg_pagesize,
         PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM, MAP_FAILED != p);
   *pages_arg = p;

   return RAMFAIL_OK;
}

ramfail_status_t ramuix_bulkalloc(char **pages_arg)
{
   char *p = NULL;

   RAMFAIL_DISALLOWZ(pages_arg);
   *pages_arg = NULL;
   RAMFAIL_CONFIRM(RAMFAIL_UNINITIALIZED, ramuix_theglobals != NULL);

   p = mmap(NULL, ramuix_theglobals->ramsysg_pagesize,
         PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM, MAP_FAILED != p);
   *pages_arg = p;

   return RAMFAIL_OK;
}

ramfail_status_t ramuix_release(char *pages_arg)
{
   RAMFAIL_DISALLOWZ(pages_arg);
   RAMFAIL_CONFIRM(RAMFAIL_UNINITIALIZED, ramuix_theglobals != NULL);
   RAMFAIL_CONFIRM(RAMFAIL_DISALLOWED,
         RAMSYS_ISPAGE(pages_arg, ramuix_theglobals));

   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM,
         0 == munmap(pages_arg, ramuix_theglobals->ramsysg_pagesize));

   return RAMFAIL_OK;
}

ramfail_status_t ramuix_mktlskey(ramuix_tlskey_t *key_arg)
{
   RAMFAIL_DISALLOWZ(key_arg);
   /* while stepping through this function, i noticed that 0 was the first
    * key given out by pthread_key_create(). consequently, i use -1 as the
    * uninitialized key value. */
   *key_arg = -1;

   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM,
         0 == pthread_key_create(key_arg, NULL));

   return RAMFAIL_OK;
}

ramfail_status_t ramuix_rmtlskey(ramuix_tlskey_t *key_arg)
{
   RAMFAIL_DISALLOWZ(key_arg);

   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM,
         0 == pthread_key_delete(*key_arg));
   *key_arg = (ramuix_tlskey_t)-1;

   return RAMFAIL_OK;
}

ramfail_status_t ramuix_stotls(ramuix_tlskey_t key_arg, void *value_arg)
{
   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM,
         0 == pthread_setspecific(key_arg, value_arg));

   return RAMFAIL_OK;
}

ramfail_status_t ramuix_mkmutex(ramuix_mutex_t *mutex_arg)
{
   RAMFAIL_DISALLOWZ(mutex_arg);

   /* TODO: protection in debug mode? */
   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM,
         0 == pthread_mutex_init(mutex_arg, NULL));

   return RAMFAIL_OK;
}

ramfail_status_t ramuix_rmmutex(ramuix_mutex_t *mutex_arg)
{
   RAMFAIL_DISALLOWZ(mutex_arg);

   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM, 0 == pthread_mutex_destroy(mutex_arg));

   return RAMFAIL_OK;
}

ramfail_status_t ramuix_waitformutex(ramuix_mutex_t *mutex_arg)
{
   RAMFAIL_DISALLOWZ(mutex_arg);

   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM, 0 == pthread_mutex_lock(mutex_arg));

   return RAMFAIL_OK;
}

ramfail_status_t ramuix_quitmutex(ramuix_mutex_t *mutex_arg)
{
   RAMFAIL_DISALLOWZ(mutex_arg);

   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM, 0 == pthread_mutex_unlock(mutex_arg));

   return RAMFAIL_OK;
}

void * ramuix_startroutine(void *arg)
{
   ramuix_startroutine_t *sr0 = NULL;
   ramuix_startroutine_t sr1 = {0};
   int32_t e = RAMFAIL_INSANE;

   assert(NULL != arg);
   sr0 = (ramuix_startroutine_t *)arg;
   /* the thread that started me is waiting on me to copy the contents of
    * *sr0* into a local structure. */
   sr1.ramuixsr_main = sr0->ramuixsr_main;
   sr1.ramuixsr_arg = sr0->ramuixsr_arg;
   /* there's no point in continuing after failing to wait on the barrier
    * because the thread that started me is likely to never continue. */
   if (RAMFAIL_OK != ramuix_waitonbarrier(&sr0->ramuixsr_barrier))
      ramfail_epicfail("failed to wait on a barrier.");
   /* after this point, *sr0* can no longer be trusted because the thread
    * that started me has likely moved on. */
   assert(NULL != sr1.ramuixsr_main);
   e = sr1.ramuixsr_main(sr1.ramuixsr_arg);
   return (void *)e;
}

ramfail_status_t ramuix_mkthread(ramuix_thread_t *thread_arg,
      ramsys_threadmain_t main_arg, void *arg_arg)
{
   pthread_t thread = -1;
   ramuix_startroutine_t sr = {0};

   RAMFAIL_DISALLOWZ(thread_arg);
   *thread_arg = -1;
   RAMFAIL_DISALLOWZ(main_arg);

   sr.ramuixsr_main = main_arg;
   sr.ramuixsr_arg = arg_arg;
   RAMFAIL_RETURN(ramuix_mkbarrier(&sr.ramuixsr_barrier, 2));
   /* TODO: does this need to be cleaned up? */
   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM,
         0 == pthread_create(&thread, NULL, &ramuix_startroutine, &sr));
   RAMFAIL_EPICFAIL(ramuix_waitonbarrier(&sr.ramuixsr_barrier));
   /* TODO: is there a race condition in pthread_barrier_wait()?
    * if i comment out the following line, i can deadlock. i shouldn't need
    * to destroy the barrier just to prevent a deadlock. that should only
    * result in a resource leak. */
   //RAMFAIL_EPICFAIL(ramuix_rmbarrier(&sr.ramuixsr_barrier));

   *thread_arg = thread;
   return RAMFAIL_OK;
}

ramfail_status_t ramuix_jointhread(ramfail_status_t *reply_arg,
      ramuix_thread_t thread_arg)
{
   void *reply = (void *)RAMFAIL_INSANE;

   RAMFAIL_DISALLOWZ(reply_arg);
   *reply_arg = RAMFAIL_INSANE;
   /* i don't know what values might be considered invalid for *thread_arg*,
    * so i cannot reliably check it. */

   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM,
         0 == pthread_join(thread_arg, &reply));

   *reply_arg = (ramfail_status_t)reply;
   return RAMFAIL_OK;
}

ramfail_status_t ramuix_mkbarrier(ramuix_barrier_t *barrier_arg,
      int capacity_arg)
{
#ifdef RAMUIX_PTHREADBARRIER
   RAMFAIL_DISALLOWZ(barrier_arg);

   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM,
         0 == pthread_barrier_init(barrier_arg, NULL, capacity_arg));

   return RAMFAIL_OK;
#else
   RAMFAIL_DISALLOWZ(barrier_arg);
   RAMFAIL_DISALLOWZ(capacity_arg);

   barrier_arg->ramlinb_capacity = capacity_arg;
   barrier_arg->ramlinb_vacancy = capacity_arg;
   barrier_arg->ramlinb_cycle = 0;
   RAMFAIL_RETURN(ramuix_mkmutex(&barrier_arg->ramlinb_mutex));
   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM,
         0 == pthread_cond_init(&barrier_arg->ramlinb_cond, NULL));

   return RAMFAIL_OK;
#endif
}

ramfail_status_t ramuix_rmbarrier(ramuix_barrier_t *barrier_arg)
{
#ifdef RAMUIX_PTHREADBARRIER
   RAMFAIL_DISALLOWZ(barrier_arg);

   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM,
         0 == pthread_barrier_destroy(barrier_arg));

   return RAMFAIL_OK;
#else
   RAMFAIL_DISALLOWZ(barrier_arg);
   RAMFAIL_CONFIRM(RAMFAIL_UNSUPPORTED,
         barrier_arg->ramlinb_capacity != barrier_arg->ramlinb_capacity);

   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM,
         0 == pthread_cond_destroy(&barrier_arg->ramlinb_cond));
   RAMFAIL_RETURN(ramuix_rmmutex(&barrier_arg->ramlinb_mutex));

   return RAMFAIL_OK;
#endif
}

ramfail_status_t ramuix_waitonbarrier(ramuix_barrier_t *barrier_arg)
{
#ifdef RAMUIX_PTHREADBARRIER
   int e = 0;

   RAMFAIL_DISALLOWZ(barrier_arg);

   e == pthread_barrier_wait(barrier_arg);
   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM,
         0 == e || PTHREAD_BARRIER_SERIAL_THREAD == e);

   return RAMFAIL_OK;
#else
   ramfail_status_t e = RAMFAIL_INSANE;

   RAMFAIL_DISALLOWZ(barrier_arg);

   RAMFAIL_RETURN(ramuix_waitformutex(&barrier_arg->ramlinb_mutex));
   e = ramlin_waitonbarrier2(barrier_arg);
   RAMFAIL_EPICFAIL(ramuix_quitmutex(&barrier_arg->ramlinb_mutex));

   return e;
#endif
}

ramfail_status_t ramlin_waitonbarrier2(ramuix_barrier_t *barrier_arg)
{
   int e = -1;
   uintptr_t cycle = 0;

   assert(barrier_arg != NULL);
   /* TODO: is it possible to test whether the mutex is locked? */

   cycle = barrier_arg->ramlinb_cycle;
   /* am i the final thread to wait at the barrier? */
   if (0 == --barrier_arg->ramlinb_vacancy)
   {
      /* i increment the cycle number to signal to the other threads that
       * they should stop polling pthread_cond_wait() and return. */
      ++barrier_arg->ramlinb_cycle;
      /* this is my opportunity to reset the vacancy counter without
       * the possibility of introducing a race condition. */
      barrier_arg->ramlinb_vacancy = barrier_arg->ramlinb_capacity;
      RAMFAIL_CONFIRM(RAMFAIL_PLATFORM,
            0 == pthread_cond_broadcast(&barrier_arg->ramlinb_cond));

      return RAMFAIL_OK;
   }
   else
   {
      /* i am not the final thread. i poll pthread_cond_wait(), waiting
       * for the cycle counter to increment, signaling that the final thread
       * has reached the barrier. */
      while (cycle == barrier_arg->ramlinb_cycle)
      {
         RAMFAIL_CONFIRM(RAMFAIL_PLATFORM,
               0 == pthread_cond_wait(&barrier_arg->ramlinb_cond,
                     &barrier_arg->ramlinb_mutex));
      }

      return RAMFAIL_OK;
   }
}

#endif /* RAMSYS_UNIX */
