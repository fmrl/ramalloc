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

#include <ramalloc/sys.h>
#include <memory.h>

#ifdef RAMSYS_WINDOWS
#  define ramsys_mkglobals ramwin_mkglobals
#elif defined(RAMSYS_UNIX)
#  define ramsys_mkglobals ramuix_mkglobals
#else
#  error <ramalloc/sys/sysdef.h> didn't provide me with a RAMSYS_/PLATFORM/ definition.
#endif /* platform check */

static ramsys_globals_t ramsys_theglobals;

ramfail_status_t ramsys_initialize()
{
   if (!ramsys_theglobals.ramsysg_initflag)
   {
      RAMFAIL_RETURN(ramsys_mkglobals(&ramsys_theglobals));

      /* i calculate the mask needed to obtain the page's address, given an address 
       *on that page. */
      ramsys_theglobals.ramsysg_pagemask = ~(ramsys_theglobals.ramsysg_pagesize - 1);

      /* something's wrong with the platform-specific modules if any fields
       * in the structure are zero. */
      RAMFAIL_CONFIRM(RAMFAIL_INSANE, ramsys_theglobals.ramsysg_granularity);
      RAMFAIL_CONFIRM(RAMFAIL_INSANE, ramsys_theglobals.ramsysg_pagesize);

      /* i depend upon the granularity being evenly divisible by the hardware
       * page size. */
      RAMFAIL_CONFIRM(RAMFAIL_UNSUPPORTED, 
            0 == (ramsys_theglobals.ramsysg_granularity % ramsys_theglobals.ramsysg_pagesize));

      ramsys_theglobals.ramsysg_initflag = 1;
   }

   return RAMFAIL_OK;
}

ramfail_status_t ramsys_getglobals(const ramsys_globals_t **globals_arg)
{
   RAMFAIL_DISALLOWZ(globals_arg);
   *globals_arg = NULL;
   RAMFAIL_CONFIRM(RAMFAIL_UNINITIALIZED, ramsys_theglobals.ramsysg_initflag);

   *globals_arg = &ramsys_theglobals;
   return RAMFAIL_OK;
}
