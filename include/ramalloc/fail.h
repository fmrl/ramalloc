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

#ifndef RAMFAIL_H_IS_INCLUDED
#define RAMFAIL_H_IS_INCLUDED

#include <ramalloc/meta.h>
#include <assert.h>
#include <stdlib.h>

typedef enum ramfail_status
{
   RAMFAIL_OK = 0,
   RAMFAIL_INSANE,         /* logic is bad; review assumptions the code makes. */
   RAMFAIL_CRT,            /* C runtime library related failure; check errno. */
   RAMFAIL_PLATFORM,       /* platform related error; e.g. check GetLastError() on Windows. */
   RAMFAIL_DISALLOWED,
   RAMFAIL_RANGE,
   RAMFAIL_RESOURCE,
   RAMFAIL_NOTFOUND,
   RAMFAIL_UNSUPPORTED,
   RAMFAIL_UNINITIALIZED,
   RAMFAIL_TRYAGAIN,
   RAMFAIL_CORRUPT,        /* a data structure failed a runtime check. */
   RAMFAIL_UNDERFLOW,
} ramfail_status_t;

typedef void (*ramfail_reporter_t)(ramfail_status_t code_arg, const char *expr_arg, 
   const char *funcn_arg, const char *filen_arg, int lineno_arg);

#define RAMFAIL_ACTIF(Condition, Action) \
   do \
   { \
      if (Condition) \
      { \
         Action; \
      } \
   } \
   while (0)

#define RAMFAIL_CONFIRM(FailCode, Condition) \
   do \
   { \
      assert(RAMFAIL_OK != (FailCode)); \
      RAMFAIL_ACTIF(!(Condition), ramfail_report((FailCode), #Condition, NULL, __FILE__, __LINE__); return (FailCode)); \
   } \
   while (0)

#define RAMFAIL_DISALLOWZ(Value) RAMFAIL_CONFIRM(RAMFAIL_DISALLOWED, 0 != (Value))

#define RMFAIL_RETURN2(Result, ResultCache) \
   do \
   { \
      const ramfail_status_t ResultCache = (Result); \
      RAMFAIL_ACTIF(RAMFAIL_OK != ResultCache, ramfail_report(ResultCache, #Result, NULL, __FILE__, __LINE__); return ResultCache); \
   } \
   while (0)

#define RAMFAIL_RETURN(Result) \
   RMFAIL_RETURN2((Result), RAMMETA_GENERATENAME(RAMFAIL_RETURN_resultcache))

#define RAMFAIL_EPICFAIL(Result) \
   RAMFAIL_ACTIF(RAMFAIL_OK != (Result), ramfail_epicfail("omgwtfbbq!"); return RAMFAIL_INSANE)

void ramfail_epicfail(const char *why_arg);
void ramfail_setreporter(ramfail_reporter_t reporter_arg);
void ramfail_report(ramfail_status_t code_arg, const char *expr_arg, const char *funcn_arg, const char *filen_arg, int lineno_arg);

#endif /* RAMFAIL_H_IS_INCLUDED */
