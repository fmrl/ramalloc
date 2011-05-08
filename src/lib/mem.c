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

#include <ramalloc/mem.h>
#include <ramalloc/sys.h>

#define RAMMEM_ISPAGE(Ptr, Mask) (0 == ((uintptr_t)(Ptr) & ~(Mask)))

typedef struct rammem_globals
{
   ramopt_malloc_t rammemg_supmalloc;
   ramopt_free_t rammemg_supfree;
   size_t rammemg_mmapgran;
   size_t rammemg_pagesize;
   uintptr_t rammemg_pagemask;
   int rammemg_initflag;
} rammem_globals_t;

static rammem_globals_t rammem_theglobals;


ramfail_status_t rammem_initialize(ramopt_malloc_t supmalloc_arg,
      ramopt_free_t supfree_arg)
{
   /* i don't support redundant calls to this function yet. */
   if (rammem_theglobals.rammemg_initflag)
      return RAMFAIL_UNSUPPORTED;
   else
   {
      if (NULL == supmalloc_arg)
         rammem_theglobals.rammemg_supmalloc = &malloc;
      else
         rammem_theglobals.rammemg_supmalloc = supmalloc_arg;
      if (NULL == supfree_arg)
         rammem_theglobals.rammemg_supfree = &free;
      else
         rammem_theglobals.rammemg_supfree = supfree_arg;

      RAMFAIL_RETURN(ramsys_pagesize(&rammem_theglobals.rammemg_pagesize));
      RAMFAIL_RETURN(ramsys_mmapgran(&rammem_theglobals.rammemg_mmapgran));
      /* something's wrong with the platform-specific modules if any fields
       * in the structure are zero. */
      RAMFAIL_CONFIRM(RAMFAIL_INSANE, rammem_theglobals.rammemg_mmapgran);
      RAMFAIL_CONFIRM(RAMFAIL_INSANE, rammem_theglobals.rammemg_pagesize);
      /* i depend upon the granularity being evenly divisible by the
       * hardware page size. */
      RAMFAIL_CONFIRM(RAMFAIL_UNSUPPORTED,
            0 == (rammem_theglobals.rammemg_mmapgran %
                  rammem_theglobals.rammemg_pagesize));

      /* i calculate the mask needed to obtain the page's address, given an
       * address on that page. */
      rammem_theglobals.rammemg_pagemask =
            ~(rammem_theglobals.rammemg_pagesize - 1);

      rammem_theglobals.rammemg_initflag = 1;
      return RAMFAIL_OK;
   }
}

void * rammem_supmalloc(size_t size_arg)
{
   if (rammem_theglobals.rammemg_initflag)
      return rammem_theglobals.rammemg_supmalloc(size_arg);
   else
   {
      ramfail_epicfail("i'm unable to invoke the supilary malloc() because rammem hasn't been initialized.");
      return NULL;
   }
}

void rammem_supfree(void *ptr_arg)
{
   if (rammem_theglobals.rammemg_initflag)
      rammem_theglobals.rammemg_supfree(ptr_arg);
   else
      ramfail_epicfail("i'm unable to invoke the supilary free() because rammem hasn't been initialized.");
}

ramfail_status_t rammem_pagesize(size_t *pgsz_arg)
{
   RAMFAIL_DISALLOWZ(pgsz_arg);
   *pgsz_arg = 0;
   RAMFAIL_CONFIRM(RAMFAIL_UNINITIALIZED,
         rammem_theglobals.rammemg_initflag);

   *pgsz_arg = rammem_theglobals.rammemg_pagesize;

   return RAMFAIL_OK;
}

ramfail_status_t rammem_mmapgran(size_t *mg_arg)
{
   RAMFAIL_DISALLOWNULL(mg_arg);
   *mg_arg = 0;
   RAMFAIL_CONFIRM(RAMFAIL_UNINITIALIZED,
         rammem_theglobals.rammemg_initflag);

   *mg_arg = rammem_theglobals.rammemg_mmapgran;

   return RAMFAIL_OK;
}

ramfail_status_t rammem_ispage(int *ispage_arg, const void *ptr_arg)
{
   RAMFAIL_DISALLOWNULL(ispage_arg);
   *ispage_arg = 0;
   RAMFAIL_CONFIRM(RAMFAIL_UNINITIALIZED,
         rammem_theglobals.rammemg_initflag);

   *ispage_arg = RAMMEM_ISPAGE(ptr_arg, rammem_theglobals.rammemg_pagemask);

   return RAMFAIL_OK;
}

ramfail_status_t rammem_getpage(char **page_arg, void *ptr_arg)
{
   RAMFAIL_DISALLOWNULL(page_arg);
   *page_arg = 0;
   RAMFAIL_DISALLOWNULL(ptr_arg);
   RAMFAIL_CONFIRM(RAMFAIL_UNINITIALIZED,
         rammem_theglobals.rammemg_initflag);

   *page_arg = (char *)((uintptr_t)ptr_arg &
         rammem_theglobals.rammemg_pagemask);

   return RAMFAIL_OK;
}

