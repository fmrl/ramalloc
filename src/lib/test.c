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

#include <ramalloc/test.h>

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
         RAMFAIL_RETURN(rammisc_swap(&p[i * size_arg], &p[j * size_arg], size_arg));
      }
   }

   return RAMFAIL_OK;
}
