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

#include <ramalloc/fail.h>
#include <stdio.h>
#include <stdlib.h>

static void ramfail_defaultreporter(ramfail_status_t code_arg, const char *expr_arg, 
   const char *funcn_arg, const char *filen_arg, int lineno_arg);

static ramfail_reporter_t ramfail_reporter = &ramfail_defaultreporter;

void ramfail_epicfail(const char *why_arg)
{
   if (!why_arg)
      why_arg = "*unspecified*";
   fprintf(stderr, "*** epic fail! %s\n", why_arg);
   abort();
}

void ramfail_setreporter(ramfail_reporter_t reporter_arg)
{
   ramfail_reporter = reporter_arg;
}

void ramfail_report(ramfail_status_t code_arg, const char *expr_arg, const char *funcn_arg, 
   const char *filen_arg, int lineno_arg)
{
   if (NULL == ramfail_reporter)
      return;
   else
      ramfail_reporter(code_arg, expr_arg, funcn_arg, filen_arg, lineno_arg);
}

void ramfail_defaultreporter(ramfail_status_t code_arg, const char *expr_arg, 
   const char *funcn_arg, const char *filen_arg, int lineno_arg)
{
   /* to my knowledge, Windows doesn't support providing the function name,
    * so i need to tolerate a NULL value for funcn_arg. */
   if (NULL == funcn_arg)
   {
      fprintf(stderr, "FAIL %d at %s, line %d: %s\n", code_arg,
            filen_arg, lineno_arg, expr_arg);
   }
   else
   {
      fprintf(stderr, "FAIL %d in %s, at %s, line %d: %s\n", code_arg,
            funcn_arg, filen_arg, lineno_arg, expr_arg);
   }
}

ramfail_status_t ramfail_accumulate(ramfail_status_t *reply_arg,
      ramfail_status_t newreply_arg)
{
   RAMFAIL_DISALLOWZ(reply_arg);

   if (RAMFAIL_OK == *reply_arg)
      *reply_arg = newreply_arg;

   return RAMFAIL_OK;
}
