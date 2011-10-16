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

static ram_reply_t ram_cast_testuinttouint();
static ram_reply_t ram_cast_testuinttoint();
static ram_reply_t ram_cast_testinttouint();
static ram_reply_t ram_cast_testinttoint();

/* TODO: when both types are unsigned (or signed), if the size of each type
 * is identical, no conversion logic should be necessary. therefore, i
 * could use CMake to check integer sizes and optimize this. */
#define RAM_CAST_MATCHING2(To, ToType, From, FromType, Tmp) \
   do \
   { \
      const ToType Tmp = ((ToType)(From)); \
      if ((FromType)Tmp == (From)) \
      { \
         *(To) = Tmp; \
         break; \
      } \
      else \
      { \
         *(To) = 0; \
         return RAM_REPLY_RANGEFAIL; \
      } \
   } \
   while (0)

#define RAM_CAST_MATCHING(To, ToType, From, FromType) \
      RAM_CAST_MATCHING2(To, ToType, From, FromType, \
            RAM_META_GENNAME(RAM_CAST_UINTTOUINT_tmp))

#define RAM_CAST_UINTTOINT2(To, ToType, ToMax, From, Tmp) \
   do \
   { \
      const uintmax_t Tmp = ((uintmax_t)(From)); \
      if (Tmp <= ((uintmax_t)(ToMax))) \
      { \
         *(To) = ((ToType)(From)); \
         break; \
      } \
      else \
      { \
         *(To) = 0; \
         return RAM_REPLY_RANGEFAIL; \
      } \
   } \
   while (0)

#define RAM_CAST_UINTTOINT(To, ToType, ToMax, From) \
      RAM_CAST_UINTTOINT2(To, ToType, ToMax, From, \
            RAM_META_GENNAME(RAM_CAST_UINTTOINT_tmp))

#define RAM_CAST_INTTOUINT2(To, ToType, ToMax, From, Tmp) \
      do \
      { \
         if ((From) >= 0) \
         { \
            const intmax_t Tmp = ((intmax_t)(From)); \
            \
            if (Tmp <= ((intmax_t)(ToMax))) \
            { \
               *(To) = ((ToType)(From)); \
               break; \
            } \
         } \
         \
         *(To) = 0; \
         return RAM_REPLY_RANGEFAIL; \
      } \
      while (0)

#define RAM_CAST_INTTOUINT(To, ToType, ToMax, From) \
      RAM_CAST_INTTOUINT2(To, ToType, ToMax, From, \
            RAM_META_GENNAME(RAM_CAST_INTTOUINT_tmp))

#define RAM_CAST_INTTOINT(To, ToType, From, FromType) \
      RAM_CAST_MATCHING(To, ToType, From, FromType)

#define RAM_CAST_UINTTOUINT(To, ToType, From, FromType) \
      RAM_CAST_MATCHING(To, ToType, From, FromType)

ram_reply_t ram_cast_ulongtochar(char *to_arg, unsigned long from_arg)
{
   RAM_FAIL_NOTNULL(to_arg);

   RAM_CAST_UINTTOINT(to_arg, char, CHAR_MAX, from_arg);

   return RAM_REPLY_OK;
}

ram_reply_t ram_cast_sizetoint(int *to_arg, size_t from_arg)
{
   RAM_FAIL_NOTNULL(to_arg);

   RAM_CAST_UINTTOINT(to_arg, int, INT_MAX, from_arg);

   return RAM_REPLY_OK;
}

ram_reply_t ram_cast_longtosize(size_t *to_arg, long from_arg)
{
   RAM_FAIL_NOTNULL(to_arg);

   RAM_CAST_INTTOUINT(to_arg, size_t, SIZE_MAX, from_arg);

   return RAM_REPLY_OK;
}

ram_reply_t ram_cast_inttosize(size_t *to_arg, int from_arg)
{
   RAM_FAIL_NOTNULL(to_arg);

   RAM_CAST_INTTOUINT(to_arg, size_t, SIZE_MAX, from_arg);

   return RAM_REPLY_OK;
}

ram_reply_t ramcast_longtouchar(unsigned char *to_arg, long from_arg)
{
   RAM_FAIL_NOTNULL(to_arg);

   RAM_CAST_INTTOUINT(to_arg, unsigned char, UCHAR_MAX, from_arg);

   return RAM_REPLY_OK;
}

ram_reply_t ram_cast_sizetolong(long *to_arg, size_t from_arg)
{
   RAM_FAIL_NOTNULL(to_arg);

   RAM_CAST_UINTTOINT(to_arg, long, LONG_MAX, from_arg);

   return RAM_REPLY_OK;
}

ram_reply_t ram_cast_sizetouint(unsigned int *to_arg, size_t from_arg)
{
   RAM_FAIL_NOTNULL(to_arg);

   RAM_CAST_UINTTOUINT(to_arg, unsigned int, from_arg, size_t);

   return RAM_REPLY_OK;
}

ram_reply_t ram_cast_ulongtouchar(unsigned char *to_arg,
      unsigned long from_arg)
{
   RAM_FAIL_NOTNULL(to_arg);

   RAM_CAST_UINTTOUINT(to_arg, unsigned char, from_arg, unsigned long);

   return RAM_REPLY_OK;
}

ram_reply_t ram_cast_ulongtouint(unsigned int *to_arg,
      unsigned long from_arg)
{
   RAM_FAIL_NOTNULL(to_arg);

   RAM_CAST_UINTTOUINT(to_arg, unsigned int, from_arg, unsigned long);

   return RAM_REPLY_OK;
}

ram_reply_t ram_cast_longtochar(char *to_arg, long from_arg)
{
   RAM_FAIL_NOTNULL(to_arg);

   RAM_CAST_INTTOINT(to_arg, char, from_arg, long);

   return RAM_REPLY_OK;
}

ram_reply_t ram_cast_test()
{
   RAM_FAIL_TRAP(ram_cast_testuinttouint());
   RAM_FAIL_TRAP(ram_cast_testuinttoint());
   RAM_FAIL_TRAP(ram_cast_testinttouint());
   RAM_FAIL_TRAP(ram_cast_testinttoint());

   return RAM_REPLY_OK;
}

ram_reply_t ram_cast_testuinttouint()
{
   unsigned char small = 0;
   unsigned long big = 0;
   ram_reply_t e = RAM_REPLY_INSANE;

   /* unsigned to unsigned conversion has two different test cases.
    * first, a success case: the value of the source variable is small
    * enough to fit into the target variable. */
   big = UCHAR_MAX;
   RAM_FAIL_TRAP(ram_cast_ulongtouchar(&small, big));
   /* second, a failure case: the value of the source variable is not small
    * enough to fit in the target variable and cannot be preserved. */
   big = ULONG_MAX;
   e = ram_cast_ulongtouchar(&small, big);
   switch (e)
   {
   default:
      RAM_FAIL_TRAP(e);
      /* @todo unreachable code here. */
      return RAM_REPLY_INSANE;
   case RAM_REPLY_OK:
      return RAM_REPLY_INSANE;
   case RAM_REPLY_RANGEFAIL:
      break;
   }

   return RAM_REPLY_OK;
}

ram_reply_t ram_cast_testuinttoint()
{
   char small = 0;
   unsigned long big = 0;
   ram_reply_t e = RAM_REPLY_INSANE;

   /* unsigned to signed conversion has three different test cases.
    * first, a success case: the value of the source variable is within
    * the range of the target variable. */
   big = (unsigned long)CHAR_MAX;
   RAM_FAIL_TRAP(ram_cast_ulongtochar(&small, big));
   /* second, a failure case: the value of the source variable is not small
    * enough to fit in the target variable's positive number space and
    * cannot be preserved. the third case is where the value of the source
    * variable. */
   big = (unsigned long)CHAR_MAX + 1;
   e = ram_cast_ulongtochar(&small, big);
   switch (e)
   {
   default:
      RAM_FAIL_TRAP(e);
      /* @todo unreachable code here. */
      return RAM_REPLY_INSANE;
   case RAM_REPLY_OK:
      return RAM_REPLY_INSANE;
   case RAM_REPLY_RANGEFAIL:
      break;
   }
   /* the third case is where the value of the source variable is the binary
    * representation of a negative value within the range of the target
    * type. this conversion cannot be allowed to succeed because the values
    * do not represent the same thing. */
   big = (unsigned long)-1;
   e = ram_cast_ulongtochar(&small, big);
   switch (e)
   {
   default:
      RAM_FAIL_TRAP(e);
      /* @todo unreachable code here. */
      return RAM_REPLY_INSANE;
   case RAM_REPLY_OK:
      return RAM_REPLY_INSANE;
   case RAM_REPLY_RANGEFAIL:
      break;
   }

   return RAM_REPLY_OK;
}

ram_reply_t ram_cast_testinttouint()
{
   unsigned char small = 0;
   long big = 0;
   ram_reply_t e = RAM_REPLY_INSANE;

   /* signed to unsigned conversion has three different test cases.
    * first, a success case: the value of the source variable is within
    * the range of the target variable. */
   big = (long)UCHAR_MAX;
   RAM_FAIL_TRAP(ramcast_longtouchar(&small, big));
   /* second, a failure case: the value of the source variable is positive
    * but is not small enough to fit within the target variable's number
    * space. */
   big = (long)UCHAR_MAX + 1;
   e = ramcast_longtouchar(&small, big);
   switch (e)
   {
   default:
      RAM_FAIL_TRAP(e);
      /* @todo unreachable code here. */
      return RAM_REPLY_INSANE;
   case RAM_REPLY_OK:
      return RAM_REPLY_INSANE;
   case RAM_REPLY_RANGEFAIL:
      break;
   }
   /* the third case is where the value of the source variable is a
    * negative value. */
   big = (long)-1;
   e = ramcast_longtouchar(&small, big);
   switch (e)
   {
   default:
      RAM_FAIL_TRAP(e);
      /* @todo unreachable code here. */
      return RAM_REPLY_INSANE;
   case RAM_REPLY_OK:
      return RAM_REPLY_INSANE;
   case RAM_REPLY_RANGEFAIL:
      break;
   }

   return RAM_REPLY_OK;
}

ram_reply_t ram_cast_testinttoint()
{
   char small = 0;
   long big = 0;
   ram_reply_t e = RAM_REPLY_INSANE;

   /* signed to signed conversion has three different test cases.
    * first, a success case: the value of the source variable is within
    * the range of the target variable. */
   big = (long)CHAR_MAX;
   RAM_FAIL_TRAP(ram_cast_longtochar(&small, big));
   /* second, a failure case: the value of the source variable is positive
    * but is not small enough to fit within the target variable's number
    * space. */
   big = (long)UCHAR_MAX + 1;
   e = ram_cast_longtochar(&small, big);
   switch (e)
   {
   default:
      RAM_FAIL_TRAP(e);
      /* @todo unreachable code here. */
      return RAM_REPLY_INSANE;
   case RAM_REPLY_OK:
      return RAM_REPLY_INSANE;
   case RAM_REPLY_RANGEFAIL:
      break;
   }
   /* second, a failure case: the value of the source variable is negative
    * and is not large enough to fit within the target variable's number
    * space. */
   big = (long)CHAR_MIN - 1;
   e = ram_cast_longtochar(&small, big);
   switch (e)
   {
   default:
      RAM_FAIL_TRAP(e);
      /* @todo unreachable code here. */
      return RAM_REPLY_INSANE;
   case RAM_REPLY_OK:
      return RAM_REPLY_INSANE;
   case RAM_REPLY_RANGEFAIL:
      break;
   }

   return RAM_REPLY_OK;
}
