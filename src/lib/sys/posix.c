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

/* <ramalloc/sys/posix.h> is included by <ramalloc/sys.h> if it's
 * appropriate for the platform. */
#include <ramalloc/sys.h>
#include <ramalloc/annotate.h>

/* this file should not compile anything if the appropriate platform
 * preprocessor definition isn't available (*RAMSYS_POSIX* in this
 * case). */
#ifdef RAMSYS_POSIX

#include <ramalloc/mem.h>
#include <errno.h>
#include <sys/mman.h>
#include <string.h>
#include <libgen.h>
/* currently, there's a bug in splint that causes it to puke if <unistd.h>
 * is included. the known workaround is to wrap it in the following #ifdef.
 * see <http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=473595> for more
 * information. */
#ifndef S_SPLINT_S
#  include <unistd.h>
#endif

static ramfail_status_t ramuix_basename2(char *dest_arg, size_t len_arg,
      const char *pathn_arg);

ramfail_status_t ramuix_initialize()
{
   return RAMFAIL_OK;
}

ramfail_status_t ramuix_pagesize(size_t *pagesz_arg)
{
   long pgsz = 0;

   RAMFAIL_DISALLOWNULL(pagesz_arg);
   *pagesz_arg = 0;

   /* on Linux, the memory mapping granularity is the page size. */
   errno = 0;
   pgsz = sysconf(_SC_PAGESIZE);
   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM, 0 == errno);
   RAMFAIL_CONFIRM(RAMFAIL_INSANE, pgsz > 0);

   *pagesz_arg = pgsz;
   return RAMFAIL_OK;
}

ramfail_status_t ramuix_mmapgran(size_t *mmapgran_arg)
{
   /* on Linux, the memory mapping granularity is the page size. */
   RAMFAIL_RETURN(ramuix_pagesize(mmapgran_arg));

   return RAMFAIL_OK;
}

ramfail_status_t ramuix_cpucount(size_t *cpucount_arg)
{
   long cpucount = 0;

   RAMFAIL_DISALLOWNULL(cpucount_arg);
   *cpucount_arg = 0;

   errno = 0;
   cpucount = sysconf(_SC_NPROCESSORS_ONLN);
   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM, 0 == errno);
   RAMFAIL_CONFIRM(RAMFAIL_INSANE, cpucount > 0);

   *cpucount_arg = cpucount;
   return RAMFAIL_OK;
}

ramfail_status_t ramuix_commit(char *page_arg)
{
   RAMANNOTATE_UNUSEDARG(page_arg);

   /* there's no difference between commit and reserve on POSIX platforms.
    * everything was already taken care of in ramuix_reserve(). */
   return RAMFAIL_OK;
}

ramfail_status_t ramuix_decommit(char *page_arg)
{
   RAMANNOTATE_UNUSEDARG(page_arg);

   /* there's no difference between commit and reserve on POSIX platforms.
    * everything will be taken care of in ramuix_release(). */
   return RAMFAIL_OK;
}

ramfail_status_t ramuix_reset(char *page_arg)
{
   /* POSIX doesn't offer an option analogous to *MEM_RESET* in Windows. */
   return ramuix_decommit(page_arg);
}

ramfail_status_t ramuix_reserve(char **pages_arg)
{
   size_t pgsz = 0;
   char *p = NULL;

   RAMFAIL_DISALLOWNULL(pages_arg);
   *pages_arg = NULL;

   RAMFAIL_RETURN(rammem_pagesize(&pgsz));
   p = mmap(NULL, pgsz,
         PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM, MAP_FAILED != p);

   *pages_arg = p;
   return RAMFAIL_OK;
}

ramfail_status_t ramuix_bulkalloc(char **pages_arg)
{
   size_t pgsz = 0;
   char *p = NULL;

   RAMFAIL_DISALLOWNULL(pages_arg);
   *pages_arg = NULL;

   RAMFAIL_RETURN(rammem_pagesize(&pgsz));
   p = mmap(NULL, pgsz,
         PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM, MAP_FAILED != p);

   *pages_arg = p;
   return RAMFAIL_OK;
}

ramfail_status_t ramuix_release(char *pages_arg)
{
   size_t pgsz = 0;
   int ispage = 0;

   RAMFAIL_DISALLOWNULL(pages_arg);
   RAMFAIL_RETURN(rammem_ispage(&ispage, pages_arg));
   RAMFAIL_CONFIRM(RAMFAIL_DISALLOWED, ispage);

   RAMFAIL_RETURN(rammem_pagesize(&pgsz));
   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM, 0 == munmap(pages_arg, pgsz));

   return RAMFAIL_OK;
}

ramfail_status_t ramuix_basename(char *dest_arg, size_t len_arg,
   const char *pathn_arg)
{
   ramfail_status_t e = RAMFAIL_INSANE;

   e = ramuix_basename2(dest_arg, len_arg, pathn_arg);
   if (RAMFAIL_OK == e)
      return RAMFAIL_OK;
   else
   {
      if (0 < len_arg)
         dest_arg[0] = '\0';
      return e;
   }
}

ramfail_status_t ramuix_basename2(char *dest_arg, size_t len_arg,
   const char *pathn_arg)
{
   char pathn[RAMSYS_PATH_MAX];
   const char *bn = NULL;

   RAMFAIL_DISALLOWNULL(dest_arg);
   RAMFAIL_DISALLOWZ(len_arg);
   RAMFAIL_DISALLOWNULL(pathn_arg);

   strncpy(pathn, pathn_arg, RAMSYS_PATH_MAX);
   pathn[RAMSYS_PATH_MAX - 1] = '\0';
   bn = basename(pathn);
   strncpy(dest_arg, bn, len_arg);
   dest_arg[len_arg - 1] = '\0';

   return RAMFAIL_OK;
}

#endif /* RAMSYS_POSIX */
