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

/* this file should not compile anything if the appropriate platform
 * preprocessor definition isn't available ('RAMSYS_WINDOWS' in this
 * case). */
#include <ramalloc/sys.h>
#ifdef RAMSYS_WINDOWS

#include <Windows.h>

static const ramsys_globals_t *ramwin_theglobals;

ramfail_status_t ramwin_mkglobals(ramsys_globals_t *globals_arg)
{
   SYSTEM_INFO sysinfo = {0};

   assert(!globals_arg->ramsysg_initflag);

   /* TODO: should i call GetNativeSystemInfo() instead? */
   GetSystemInfo(&sysinfo);
   globals_arg->ramsysg_granularity = sysinfo.dwAllocationGranularity;
   globals_arg->ramsysg_pagesize = sysinfo.dwPageSize;

   /* i'm allowed to cache 'globals_arg' since it's a singleton. */
   ramwin_theglobals = globals_arg;

   return RAMFAIL_OK;
}

ramfail_status_t ramwin_commit(char *page_arg)
{
   RAMFAIL_DISALLOWZ(page_arg);
   RAMFAIL_RETURN(ramsys_initialize());
   RAMFAIL_CONFIRM(RAMFAIL_DISALLOWED, RAMSYS_ISPAGE(page_arg, ramwin_theglobals));
   
   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM, 
      VirtualAlloc(page_arg, ramwin_theglobals->ramsysg_pagesize, MEM_COMMIT, 
      PAGE_READWRITE) == page_arg);

   return RAMFAIL_OK;
}

ramfail_status_t ramwin_decommit(char *page_arg)
{
   RAMFAIL_DISALLOWZ(page_arg);
   RAMFAIL_RETURN(ramsys_initialize());
   RAMFAIL_CONFIRM(RAMFAIL_DISALLOWED, RAMSYS_ISPAGE(page_arg, ramwin_theglobals));
   
   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM,
         VirtualFree(page_arg, ramwin_theglobals->ramsysg_pagesize, MEM_DECOMMIT));

   return RAMFAIL_OK;
}

ramfail_status_t ramwin_reset(char *page_arg)
{
   RAMFAIL_DISALLOWZ(page_arg);
   RAMFAIL_RETURN(ramsys_initialize());
   RAMFAIL_CONFIRM(RAMFAIL_DISALLOWED, RAMSYS_ISPAGE(page_arg, ramwin_theglobals));

   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM, 
      VirtualAlloc(page_arg, ramwin_theglobals->ramsysg_pagesize, MEM_RESET, 
      PAGE_NOACCESS) == page_arg);

   return RAMFAIL_OK;
}

ramfail_status_t ramwin_reserve(char **pages_arg)
{
   RAMFAIL_DISALLOWZ(pages_arg);
   *pages_arg = NULL;
   RAMFAIL_RETURN(ramsys_initialize());

   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM, 
      *pages_arg = (char *)VirtualAlloc(NULL, ramwin_theglobals->ramsysg_granularity, 
      MEM_RESERVE, PAGE_NOACCESS));

   return RAMFAIL_OK;
}

ramfail_status_t ramwin_bulkalloc(char **pages_arg)
{
   RAMFAIL_DISALLOWZ(pages_arg);
   *pages_arg = NULL;
   RAMFAIL_RETURN(ramsys_initialize());

   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM, 
      *pages_arg = (char *)VirtualAlloc(NULL, ramwin_theglobals->ramsysg_granularity, 
      MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));

   return RAMFAIL_OK;
}

ramfail_status_t ramwin_release(char *pages_arg)
{
   RAMFAIL_DISALLOWZ(pages_arg);
   RAMFAIL_RETURN(ramsys_initialize());
   RAMFAIL_CONFIRM(RAMFAIL_DISALLOWED, RAMSYS_ISPAGE(pages_arg, ramwin_theglobals));
   
   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM,
         VirtualFree(pages_arg, 0, MEM_RELEASE));

   return RAMFAIL_OK;
}

ramfail_status_t ramwin_mktlskey(ramwin_tlskey_t *key_arg)
{
   ramwin_tlskey_t k = RAMWIN_NILTLSKEY;

   RAMFAIL_DISALLOWZ(key_arg);
   *key_arg = RAMWIN_NILTLSKEY;

   k = TlsAlloc();
   if (TLS_OUT_OF_INDEXES == k)
      return RAMFAIL_RESOURCE;
   else
   {
      *key_arg = k;
      return RAMFAIL_OK;
   }
}

ramfail_status_t ramwin_rmtlskey(ramwin_tlskey_t key_arg)
{
   RAMFAIL_CONFIRM(RAMFAIL_DISALLOWED, key_arg != RAMWIN_NILTLSKEY);

   if (TlsFree(key_arg))
      return RAMFAIL_OK;
   else
      return RAMFAIL_PLATFORM;
}

ramfail_status_t ramwin_rcltls(void **value_arg, ramwin_tlskey_t key_arg)
{
   void *p = NULL;

   RAMFAIL_DISALLOWZ(value_arg);
   *value_arg = NULL;
   RAMFAIL_CONFIRM(RAMFAIL_DISALLOWED, key_arg != RAMWIN_NILTLSKEY);

   p = TlsGetValue(key_arg);
   /* NULL is an ambiguous return value. i must check to see if an error
    * occurres to be certain. */
   /* TODO: TlsGetValue() doesn't check whether key_arg is valid, so i'd need
    * to implement this check (or ensure it's validity) myself. */
   if (p || ERROR_SUCCESS == GetLastError())
   {
      *value_arg = p;
      return RAMFAIL_OK;
   }
   else
      return RAMFAIL_PLATFORM;
}

ramfail_status_t ramwin_stotls(ramwin_tlskey_t key_arg, void *value_arg)
{
   RAMFAIL_CONFIRM(RAMFAIL_DISALLOWED, key_arg != RAMWIN_NILTLSKEY);
   RAMFAIL_DISALLOWZ(value_arg);

   if (TlsSetValue(key_arg, value_arg))
      return RAMFAIL_OK;
   else
      return RAMFAIL_PLATFORM;
}

ramfail_status_t ramwin_mkmutex(ramwin_mutex_t *mutex_arg)
{
   RAMFAIL_DISALLOWZ(mutex_arg);

   InitializeCriticalSection(mutex_arg);

   return RAMFAIL_OK;
}

ramfail_status_t ramwin_rmmutex(ramwin_mutex_t *mutex_arg)
{
   RAMFAIL_DISALLOWZ(mutex_arg);

   DeleteCriticalSection(mutex_arg);

   return RAMFAIL_OK;
}


ramfail_status_t ramwin_waitformutex(ramwin_mutex_t *mutex_arg)
{
   RAMFAIL_DISALLOWZ(mutex_arg);

   EnterCriticalSection(mutex_arg);

   return RAMFAIL_OK;
}

ramfail_status_t ramwin_quitmutex(ramwin_mutex_t *mutex_arg)
{
   RAMFAIL_DISALLOWZ(mutex_arg);

   LeaveCriticalSection(mutex_arg);

   return RAMFAIL_OK;
}

ramfail_status_t ramwin_mkthread(ramsys_threadmain_t main_arg, void *arg_arg)
{
   HANDLE thread = NULL;

   RAMFAIL_DISALLOWZ(main_arg);

   /* TODO: casting 'main_arg' to LPTHREAD_START_ROUTINE simplifies the code but it
    * sacrifies type safety. if i end up needing a more sophisticated threading
    * interface, i'll consider changing this. */
   thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main_arg, arg_arg, 0, NULL);
   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM, NULL != thread);

   return RAMFAIL_OK;
}

ramfail_status_t ramwin_mkbarrier(ramwin_barrier_t *barrier_arg, int capacity_arg)
{
   RAMFAIL_DISALLOWZ(barrier_arg);

   memset(barrier_arg, 0, sizeof(*barrier_arg));

   barrier_arg->ramwinb_event = CreateEvent(NULL, FALSE, FALSE, NULL);
   RAMFAIL_CONFIRM(RAMFAIL_PLATFORM, NULL != barrier_arg->ramwinb_event);
   barrier_arg->ramwinb_capacity = capacity_arg;
   barrier_arg->ramwinb_vacancy = capacity_arg;

   return RAMFAIL_OK;
}

ramfail_status_t ramwin_waitonbarrier(ramwin_barrier_t *barrier_arg)
{
   LONG n = 0;

   RAMFAIL_DISALLOWZ(barrier_arg);
   RAMFAIL_CONFIRM(RAMFAIL_UNINITIALIZED, barrier_arg->ramwinb_capacity > 0);

   n = InterlockedDecrement(&barrier_arg->ramwinb_vacancy);
   if (n > 0)
   {
      DWORD result = WAIT_FAILED;

      result = WaitForSingleObject(barrier_arg->ramwinb_event, INFINITE);
      RAMFAIL_CONFIRM(RAMFAIL_PLATFORM, WAIT_OBJECT_0 == result);
   }
   else if (0 == n)
   {
      /* if i'm the last one waiting on the barrier, i set it to the signaled state,
       * rather than wait on it.*/
      RAMFAIL_CONFIRM(RAMFAIL_PLATFORM, SetEvent(barrier_arg->ramwinb_event));
   }
   else
   {
      assert(n < 0);

      /* if n < 0, then it means that i should inform the caller of an underflow. */
      return RAMFAIL_UNDERFLOW;
   }

   return RAMFAIL_OK;
}

#endif /* RAMSYS_WINDOWS */
