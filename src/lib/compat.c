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

#include <ramalloc/compat.h>
#include <ramalloc/default.h>
#include <ramalloc/mem.h>
#include <memory.h>
#include <string.h>

void * ramcompat_malloc(size_t size_arg)
{
   if (0 == size_arg)
      return rammem_supmalloc(size_arg);
   else
   {
      ramfail_status_t e = RAMFAIL_INSANE;
      void *p = NULL;

      e = ramdefault_acquire(&p, size_arg);
      switch (e)
      {
      default:
         return NULL;
      case RAMFAIL_OK:
         return p;
      case RAMFAIL_RANGE:
         /* ramalloc will return RAMFAIL_RANGE if the allocator cannot accomidate
          * an object of size 'size_arg' (i.e. too big or too small). i can defer to
          * the supplimental allocator for this. */
         return rammem_supmalloc(size_arg);
      }
   }
}

void ramcompat_free(void *ptr_arg)
{
   /* free() does nothing if the pointer is NULL, so i need to emulate that. */
   if (NULL == ptr_arg)
      return;
   else
   {
      ramfail_status_t e = RAMFAIL_INSANE;
      void *p = NULL;
      size_t sz = 0;

      e = ramdefault_query(&sz, ptr_arg);
      switch (e)
      {
      default:
         /* i don't have any other avenue through which i can report an error. */
         ramfail_epicfail("i got an unexpected eror from ramdefault_query().");
         return;
      case RAMFAIL_OK:
         e = ramdefault_discard(ptr_arg);
         if (RAMFAIL_OK != e)
            ramfail_epicfail("i got an unexpected eror from ramdefault_discard().");
         return;
      case RAMFAIL_NOTFOUND:
         /* ramalloc will return RAMFAIL_NOTFOUND if ptr_arg was allocated with a different
          * allocator. */
         rammem_supfree(ptr_arg);
         return;
      }
   }
}

void * ramcompat_calloc(size_t count_arg, size_t size_arg)
{
   void *p = NULL;
   size_t sz = 0;

   sz = count_arg * size_arg;
   p = ramcompat_malloc(sz);
   if (NULL == p)
      return NULL;
   else
   {
      memset(p, 0, sz);
      return p;
   }
}
