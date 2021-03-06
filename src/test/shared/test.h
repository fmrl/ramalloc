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

#ifndef RAMTEST_H_IS_INCLUDED
#define RAMTEST_H_IS_INCLUDED

#include <ramalloc/fail.h>
#include <ramalloc/stdint.h>
#include <ramalloc/compiler.h>
#include <stdio.h>

typedef struct ramtest_allocdesc
{
   char *ramtestad_ptr;
   size_t ramtestad_sz;
   void *ramtestad_pool;
} ramtest_allocdesc_t;

typedef ram_reply_t (*ramtest_acquire_t)(ramtest_allocdesc_t *desc_arg,
      size_t size_arg, void *extra_arg, size_t threadidx_arg);
typedef ram_reply_t
      (*ramtest_release_t)(ramtest_allocdesc_t *desc_arg);
typedef ram_reply_t (*ramtest_query_t)(void **pool_arg,
      size_t *size_arg, void *ptr_arg, void *extra_arg);
typedef ram_reply_t (*ramtest_flush_t)(void *extra_arg,
      size_t threadidx_arg);
typedef ram_reply_t (*ramtest_check_t)(void *extra_arg,
      size_t threadidx_arg);

typedef struct ramtest_params
{
   void *ramtestp_extra;
   size_t ramtestp_minsize;
   size_t ramtestp_maxsize;
   int ramtestp_mallocchance;
   size_t ramtestp_threadcount;
   size_t ramtestp_alloccount;
   ramtest_acquire_t ramtestp_acquire;
   ramtest_release_t ramtestp_release;
   ramtest_query_t ramtestp_query;
   ramtest_flush_t ramtestp_flush;
   ramtest_check_t ramtestp_check;
   int ramtestp_dryrun;
   unsigned int ramtestp_rngseed;
   int ramtestp_userngseed;
   int ramtestp_nofill;
} ramtest_params_t;

ram_reply_t ramtest_randuint32(uint32_t *result_arg, uint32_t n0_arg,
      uint32_t n1_arg);
ram_reply_t ramtest_randint32(int32_t *result_arg, int32_t n0_arg,
      int32_t n1_arg);
ram_reply_t ramtest_shuffle(void *array_arg, size_t size_arg,
      size_t count_arg);

ram_reply_t ramtest_test(const ramtest_params_t *params_arg);

ram_reply_t ramtest_defaultthreadcount(size_t *count_arg);
ram_reply_t ramtest_maxthreadcount(size_t *count_arg);

/*@printflike@*/
RAMSYS_PRINTFDECL(
      ram_reply_t ramtest_fprintf(size_t *count_arg, FILE *file_arg,
            const char *fmt_arg, ...),
      3, 4);

#endif /* RAMTEST_H_IS_INCLUDED */
