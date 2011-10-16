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

/* <ramalloc/sys/linux.h> is included by <ramalloc/sys.h> if it's
 * appropriate for the platform. */
#include <ramalloc/sys.h>

/* this file should not compile anything if the appropriate platform
 * preprocessor definition isn't available ('RAMSYS_LINUX' in this
 * case). */
#ifdef RAMSYS_LINUX

#include <ramalloc/mtx.h>

static ram_reply_t
   ramlin_waitonbarrier2(ramlin_barrier_t *barrier_arg);

ram_reply_t ramlin_mkbarrier(ramlin_barrier_t *barrier_arg,
      size_t capacity_arg)
{
   RAM_FAIL_NOTNULL(barrier_arg);
   RAM_FAIL_NOTZERO(capacity_arg);

   barrier_arg->ramlinb_capacity = capacity_arg;
   barrier_arg->ramlinb_vacancy = capacity_arg;
   barrier_arg->ramlinb_cycle = 0;
   RAM_FAIL_TRAP(ramuix_mkmutex(&barrier_arg->ramlinb_mutex));
   RAM_FAIL_EXPECT(RAM_REPLY_APIFAIL,
         0 == pthread_cond_init(&barrier_arg->ramlinb_cond, NULL));

   return RAM_REPLY_OK;
}

ram_reply_t ramlin_rmbarrier(ramlin_barrier_t *barrier_arg)
{
   RAM_FAIL_NOTNULL(barrier_arg);
   /* i don't allow destruction of the barrier while it's in use. */
   RAM_FAIL_EXPECT(RAM_REPLY_UNSUPPORTED,
         barrier_arg->ramlinb_vacancy == barrier_arg->ramlinb_capacity);

   RAM_FAIL_EXPECT(RAM_REPLY_APIFAIL,
         0 == pthread_cond_destroy(&barrier_arg->ramlinb_cond));
   RAM_FAIL_TRAP(rammtx_rmmutex(&barrier_arg->ramlinb_mutex));

   return RAM_REPLY_OK;
}

ram_reply_t ramlin_waitonbarrier(ramlin_barrier_t *barrier_arg)
{
   ram_reply_t e = RAM_REPLY_INSANE;

   RAM_FAIL_NOTNULL(barrier_arg);

   RAM_FAIL_TRAP(rammtx_wait(&barrier_arg->ramlinb_mutex));
   e = ramlin_waitonbarrier2(barrier_arg);
   /* there's no point in continuing if i fail to release the mutex. */
   /* TODO: ..._quitmutex() looks like it might be a candidate to succeed-
    * or-die. */
   RAM_FAIL_PANIC(rammtx_quit(&barrier_arg->ramlinb_mutex));

   return e;
}

ram_reply_t ramlin_waitonbarrier2(ramlin_barrier_t *barrier_arg)
{
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
      RAM_FAIL_EXPECT(RAM_REPLY_APIFAIL,
            0 == pthread_cond_broadcast(&barrier_arg->ramlinb_cond));

      return RAM_REPLY_OK;
   }
   else
   {
      /* i am not the final thread. i poll pthread_cond_wait(), waiting
       * for the cycle counter to increment, signaling that the final thread
       * has reached the barrier. */
      while (cycle == barrier_arg->ramlinb_cycle)
      {
         RAM_FAIL_EXPECT(RAM_REPLY_APIFAIL,
               0 == pthread_cond_wait(&barrier_arg->ramlinb_cond,
                     &barrier_arg->ramlinb_mutex));
      }

      return RAM_REPLY_OK;
   }
}

#endif /* RAMSYS_LINUX */

