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

#include <ramalloc/cast.h>
#include <ramalloc/stdint.h>
#include <ramalloc/meta.h>
#include <limits.h>

/* TODO: i need to write a test for this module */

#define RAMCAST_NUMERICCAST2(ToVar, ToType, FromValue, FromType, FromVar) \
   do \
   { \
      FromType FromVar = (FromValue); \
      (ToVar) = (ToType)from_arg; \
      if (((FromType)(ToVar)) != FromVar) \
      { \
         (ToVar) = 0; \
         return RAMFAIL_RANGE; \
      } \
   } \
   while (0)

 #define RAMCAST_NUMERICCAST(ToVar, ToType, FromValue, FromType) \
      RAMMETACAST_NUMERICCAST2(ToType, ToVar, FromType, FromValue, \
      RAMMETA_GENERATENAME(RAMCASE_NUMERICCAST_fromcache_)

ramfail_status_t ramcast_sizetoint(int *to_arg, size_t from_arg)
{
   uintmax_t n = (uintmax_t)from_arg;

   RAMFAIL_DISALLOWNULL(to_arg);

   if (n <= (uintmax_t)INT_MAX)
   {
      *to_arg = (int)from_arg;
      return RAMFAIL_OK;
   }
   else
   {
      *to_arg = 0;
      return RAMFAIL_RANGE;
   }
}

ramfail_status_t ramcast_longtosize(size_t *to_arg, long from_arg)
{
   RAMFAIL_DISALLOWNULL(to_arg);

   if (from_arg >= 0)
   {
      intmax_t n = (intmax_t)from_arg;

      if (n <= (intmax_t)SIZE_MAX)
      {
         *to_arg = (size_t)from_arg;
         return RAMFAIL_OK;
      }
   }

   *to_arg = 0;
   return RAMFAIL_RANGE;
}

ramfail_status_t ramcast_sizetouint(unsigned int *to_arg, size_t from_arg)
{
   unsigned int n;

   RAMFAIL_DISALLOWNULL(to_arg);

   /* TODO: both types are unsigned, so if the size of each type is
    * identical, no conversion logic should be necessary. therefore, i
    * could use CMake to check integer sizes and optimize this. */
   n = (unsigned int)from_arg;
   if ((size_t)n == from_arg)
   {
      *to_arg = n;
      return RAMFAIL_OK;
   }
   else
   {
      *to_arg = 0;
      return RAMFAIL_RANGE;
   }
}
