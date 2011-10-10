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

/* <ramalloc/sys/pthreads.h> is included by <ramalloc/sys.h> if it's
 * appropriate for the platform. */
#include <ramalloc/sys.h>

/* this file should not compile anything if the appropriate platform
 * preprocessor definition isn't available ('RAMSYS_PTHREADS' in this
 * case). */
#ifdef RAMSYS_PTHREADS

#include <ramalloc/barrier.h>
#include <ramalloc/cast.h>
#include <errno.h>

typedef struct ramuix_startroutine
{
   ramsys_threadmain_t ramuixsr_main;
   void *ramuixsr_userarg;
   rambarrier_barrier_t ramuixsr_barrier;
} ramuix_startroutine_t;

static void * ramuix_startroutine(void *sr_arg);

ram_reply_t ramuix_mktlskey(ramuix_tlskey_t *key_arg)
{
   RAM_FAIL_NOTNULL(key_arg);
   /* while stepping through this function, i noticed that 0 was the first
    * key given out by pthread_key_create(). consequently, i use -1 as the
    * uninitialized key value. */
   *key_arg = -1;

   RAM_FAIL_EXPECT(RAM_REPLY_APIFAIL,
         0 == pthread_key_create(key_arg, NULL));

   return RAM_REPLY_OK;
}

ram_reply_t ramuix_rmtlskey(ramuix_tlskey_t key_arg)
{
   RAM_FAIL_EXPECT(RAM_REPLY_APIFAIL,
         0 == pthread_key_delete(key_arg));

   return RAM_REPLY_OK;
}

ram_reply_t ramuix_rcltls(void **tls_arg, ramuix_tlskey_t key_arg)
{
   RAM_FAIL_NOTNULL(tls_arg);

   *tls_arg = pthread_getspecific(key_arg);
   return RAM_REPLY_OK;
}


ram_reply_t ramuix_stotls(ramuix_tlskey_t key_arg, void *value_arg)
{
   RAM_FAIL_EXPECT(RAM_REPLY_APIFAIL,
         0 == pthread_setspecific(key_arg, value_arg));

   return RAM_REPLY_OK;
}

ram_reply_t ramuix_mkmutex(ramuix_mutex_t *mutex_arg)
{
   RAM_FAIL_NOTNULL(mutex_arg);

   /* TODO: protection in debug mode? */
   RAM_FAIL_EXPECT(RAM_REPLY_APIFAIL,
         0 == pthread_mutex_init(mutex_arg, NULL));

   return RAM_REPLY_OK;
}

ram_reply_t ramuix_rmmutex(ramuix_mutex_t *mutex_arg)
{
   RAM_FAIL_NOTNULL(mutex_arg);

   RAM_FAIL_EXPECT(RAM_REPLY_APIFAIL, 0 == pthread_mutex_destroy(mutex_arg));

   return RAM_REPLY_OK;
}

ram_reply_t ramuix_waitformutex(ramuix_mutex_t *mutex_arg)
{
   RAM_FAIL_NOTNULL(mutex_arg);

   RAM_FAIL_EXPECT(RAM_REPLY_APIFAIL, 0 == pthread_mutex_lock(mutex_arg));

   return RAM_REPLY_OK;
}

ram_reply_t ramuix_quitmutex(ramuix_mutex_t *mutex_arg)
{
   RAM_FAIL_NOTNULL(mutex_arg);

   RAM_FAIL_EXPECT(RAM_REPLY_APIFAIL, 0 == pthread_mutex_unlock(mutex_arg));

   return RAM_REPLY_OK;
}

void * ramuix_startroutine(void *sr_arg)
{
   ramuix_startroutine_t *orig = NULL;
   ramuix_startroutine_t cpy = {0};
   ram_reply_t e = RAM_REPLY_INSANE;

   assert(NULL != sr_arg);
   orig = (ramuix_startroutine_t *)sr_arg;
   /* the thread that started me is waiting on me to copy the contents of
    * *sr0* into a local structure. */
   cpy.ramuixsr_main = orig->ramuixsr_main;
   cpy.ramuixsr_userarg = orig->ramuixsr_userarg;
   /* there's no point in continuing after failing to wait on the barrier
    * because the thread that started me is likely to never continue. */
   if (RAM_REPLY_OK != rambarrier_wait(&orig->ramuixsr_barrier))
      ram_fail_panic("failed to wait on a barrier.");
   /* after this point, *orig* can no longer be trusted because the thread
    * that started me has likely moved on. */
   assert(NULL != cpy.ramuixsr_main);
   e = cpy.ramuixsr_main(cpy.ramuixsr_userarg);
   return (void *)e;
}

ram_reply_t ramuix_mkthread(ramuix_thread_t *thread_arg,
      ramsys_threadmain_t main_arg, void *arg_arg)
{
   pthread_t thread;
   ramuix_startroutine_t sr = {0};

   RAM_FAIL_NOTNULL(thread_arg);
   *thread_arg = -1;
   RAM_FAIL_NOTNULL(main_arg);

   sr.ramuixsr_main = main_arg;
   sr.ramuixsr_userarg = arg_arg;
   RAM_FAIL_TRAP(rambarrier_mkbarrier(&sr.ramuixsr_barrier, 2));
   /* TODO: does this need to be cleaned up? */
   RAM_FAIL_EXPECT(RAM_REPLY_APIFAIL,
         0 == pthread_create(&thread, NULL, &ramuix_startroutine, &sr));
   RAM_FAIL_PANIC(rambarrier_wait(&sr.ramuixsr_barrier));
#if !RAM_WANT_NPTLDEADLOCK
   /* there appears to be a race condition in pthread_barrier_wait().
    * if the following line is removed, i deadlock rather reliably after
    * hitting pthread_barrier_wait() 2-3 times. i shouldn't need to destroy
    * the barrier to prevent a deadlock. that should only result in a
    * resource leak. i produced the deadlock using Ubuntu Linux 10.10
    * on a Dell Mini 9 with 512M or memory and 4G of swap. */
   RAM_FAIL_PANIC(rambarrier_rmbarrier(&sr.ramuixsr_barrier));
#endif /* !RAM_WANT_NPTLDEADLOCK */

   *thread_arg = thread;
   return RAM_REPLY_OK;
}

ram_reply_t ramuix_jointhread(ram_reply_t *reply_arg,
      ramuix_thread_t thread_arg)
{
   void *reply = (void *)RAM_REPLY_INSANE;

   RAM_FAIL_NOTNULL(reply_arg);
   *reply_arg = RAM_REPLY_INSANE;
   /* i don't know what values might be considered invalid for *thread_arg*,
    * so i cannot reliably check it. */

   RAM_FAIL_EXPECT(RAM_REPLY_APIFAIL,
         0 == pthread_join(thread_arg, &reply));

   *reply_arg = (ram_reply_t)reply;
   return RAM_REPLY_OK;
}

ram_reply_t ramuix_mkbarrier(ramuix_barrier_t *barrier_arg,
      size_t capacity_arg)
{
   unsigned int capacity;

   RAM_FAIL_NOTNULL(barrier_arg);

   RAM_FAIL_TRAP(ram_cast_sizetouint(&capacity, capacity_arg));
   RAM_FAIL_EXPECT(RAM_REPLY_APIFAIL,
         0 == pthread_barrier_init(barrier_arg, NULL, capacity));

   return RAM_REPLY_OK;
}

ram_reply_t ramuix_rmbarrier(ramuix_barrier_t *barrier_arg)
{
   RAM_FAIL_NOTNULL(barrier_arg);

   RAM_FAIL_EXPECT(RAM_REPLY_APIFAIL,
         0 == pthread_barrier_destroy(barrier_arg));

   return RAM_REPLY_OK;
}

ram_reply_t ramuix_waitonbarrier(ramuix_barrier_t *barrier_arg)
{
   /* pthread_barrier_wait() will not return EINTR. */
   int e = EINTR;

   RAM_FAIL_NOTNULL(barrier_arg);

   e = pthread_barrier_wait(barrier_arg);
   RAM_FAIL_EXPECT(RAM_REPLY_APIFAIL,
         0 == e || PTHREAD_BARRIER_SERIAL_THREAD == e);

   return RAM_REPLY_OK;
}

#endif /* RAMSYS_PTHREADS */

