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

#include <ramalloc/ramalloc.h>
#include <ramalloc/compat.h>
#include <ramalloc/sys.h>
#include <ramalloc/mem.h>

#define SMALL_SIZE 4
#define LARGE_SIZE (1024 * 100)

static ramfail_status_t malloctest(size_t size_arg);
static ramfail_status_t calloctest(size_t count_arg, size_t size_arg);

int main()
{
   size_t pgsz = 0;

   RAMFAIL_CONFIRM(-1, RAMFAIL_OK == ramalloc_initialize(NULL, NULL));
   RAMFAIL_CONFIRM(-2, RAMFAIL_OK == rammem_pagesize(&pgsz));
   RAMFAIL_CONFIRM(1, RAMFAIL_OK == malloctest(0));
   RAMFAIL_CONFIRM(2, RAMFAIL_OK == malloctest(SMALL_SIZE));
   RAMFAIL_CONFIRM(3, RAMFAIL_OK == malloctest(LARGE_SIZE));
   RAMFAIL_CONFIRM(3, RAMFAIL_OK == malloctest((pgsz / (RAMOPT_MINDENSITY - 1))));
   RAMFAIL_CONFIRM(4, RAMFAIL_OK == calloctest(2, SMALL_SIZE));
   RAMFAIL_CONFIRM(5, RAMFAIL_OK == calloctest(2, LARGE_SIZE));

   return 0;
}

ramfail_status_t malloctest(size_t size_arg)
{
   char *p = NULL;

   p = ramcompat_malloc(size_arg);
   RAMFAIL_CONFIRM(RAMFAIL_INSANE, p != NULL);
   ramcompat_free(p);

   return RAMFAIL_OK;
}

ramfail_status_t calloctest(size_t count_arg, size_t size_arg)
{
   char *p = NULL;

   p = ramcompat_calloc(count_arg, size_arg);
   RAMFAIL_CONFIRM(RAMFAIL_INSANE, p != NULL);
   ramcompat_free(p);

   return RAMFAIL_OK;
}
