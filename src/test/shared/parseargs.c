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

#include "parseargs.h"
#include <ramalloc/stdint.h>
#include <ramalloc/sys.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <errno.h>

static ramfail_status_t parseargs2(ramtest_params_t *params_arg,
      int argc_arg, char *argv_arg[]);
static ramfail_status_t parseulong(unsigned long *n_arg,
      const char *str_arg);
static ramfail_status_t parselong(long *n_arg, const char *str_arg);
static ramfail_status_t parsealloccount(size_t *alloccount_arg);
static ramfail_status_t parsethreadcount(size_t *threadcount_arg);
static ramfail_status_t parsemallocchance(int *mallocchance_arg);
static ramfail_status_t parseminsize(size_t *minsize_arg);
static ramfail_status_t parsemaxsize(size_t *maxsize_arg);
static ramfail_status_t parserngseed(unsigned int *rngseed_arg);


static const char usagemsg[] =
      "%s: tests a ramalloc pool.\n"
      "\n"
      "usage:\n"
      "\n"
      "%s [options]\n"
      "\n"
      "options:\n"
      "\n"
      "-a <n> or --alocations=<n>\n"
      "\tspecify the number of allocations to test (also specifies\n"
      "\ttest length).\n"
      "\n"
      "-p <n> or --parallelize=<n>\n"
      "\tparallelize the test (1 specifies no parallelization).\n"
      "\n"
      "-m <n> or --malloc=<n>\n"
      "\tspecify the percentage of allocations that should use malloc()\n"
      "\tinstead of the pool.\n"
      "\n"
      "-s <n> or --smallest=<n>\n"
      "\tspecify the smallest allocation request that should be made, in\n"
      "\tbytes.\n"
      "\n"
      "-l <n> or --largest=<n>\n"
      "\tspecify the largest allocation request that should be made, in\n"
      "\tbytes.\n"
      "\n"
      "-n or --dry-run\n"
      "\ti will describe the test but i will not run it.\n"
      "\n"
      "-S or --rng-seed\n"
      "\tspecifies the random number generator's seed for non-\n"
      "\tparallelized tests.\n";


void usage(ramfail_status_t exit_arg, int argc_arg, char *argv_arg[])
{
   char bn[RAMSYS_PATH_MAX];

   /* i expect argv_arg to contain at least one element. */
   if (1 > argc_arg)
      ramfail_epicfail("i expect at least one argument.");
   if (NULL == argv_arg)
      ramfail_epicfail("i encountered an unexpected NULL pointer.");

   if (RAMFAIL_OK != ramsys_basename(bn, RAMSYS_PATH_MAX, argv_arg[0]))
      ramfail_epicfail("i failed to get the basename of the process.");

   fprintf(stderr, usagemsg, bn, bn);
   exit(exit_arg);
}

ramfail_status_t parseargs(ramtest_params_t *params_arg, int argc_arg,
      char *argv_arg[])
{
   ramfail_status_t e = RAMFAIL_INSANE;

   RAMFAIL_DISALLOWNULL(params_arg);
   e = parseargs2(params_arg, argc_arg, argv_arg);
   if (RAMFAIL_OK != e)
      memset(params_arg, 0, sizeof(*params_arg));

   return e;
}

ramfail_status_t parseargs2(ramtest_params_t *params_arg, int argc_arg,
      char *argv_arg[])
{
   int c = 0, i = 0;
   static struct option longopts[] =
   {
         { "allocations", 1, 0, 'a' },
         { "parallelize", 1, 0, 'p' },
         { "malloc", 1, 0, 'm' },
         { "smallest", 1, 0, 's' },
         { "largest", 1, 0, 'l' },
         { "rng-seed", 1, 0, 'S' },
         { "help", 0, 0, 'h' },
         { "dry-run", 0, 0, 'n' },
         { 0, 0, 0, 0 }
   };

   assert(params_arg != NULL);
   RAMFAIL_CONFIRM(RAMFAIL_DISALLOWED, argc_arg >= 0);
   RAMFAIL_DISALLOWNULL(argv_arg);

   while (-1 != (c = getopt_long(argc_arg, argv_arg,
         "a:p:m:s:l:n", longopts, &i)))
   {
      switch (c)
      {
      default:
         /* this would be where options that don't have a short name would
          * be processed. */
         ramfail_epicfail("unexpected option.");
         break;

      case 'a':
      {
         ramfail_status_t e = RAMFAIL_INSANE;

         e = parsealloccount(&params_arg->ramtestp_alloccount);
         switch (e)
         {
         default:
            RAMFAIL_RETURN(e);
         case RAMFAIL_OK:
            break;
         case RAMFAIL_INPUT:
            return e;
         }
         break;
      }

      case 'p':
      {
         ramfail_status_t e = RAMFAIL_INSANE;

         e = parsethreadcount(&params_arg->ramtestp_threadcount);
         switch (e)
         {
         default:
            RAMFAIL_RETURN(e);
         case RAMFAIL_OK:
            break;
         case RAMFAIL_INPUT:
            return e;
         }
         break;
      }

      case 'm':
      {
         ramfail_status_t e = RAMFAIL_INSANE;

         e = parsemallocchance(&params_arg->ramtestp_mallocchance);
         switch (e)
         {
         default:
            RAMFAIL_RETURN(e);
         case RAMFAIL_OK:
            break;
         case RAMFAIL_INPUT:
            return e;
         }
         break;
      }

      case 's':
      {
         ramfail_status_t e = RAMFAIL_INSANE;

         e = parseminsize(&params_arg->ramtestp_minsize);
         switch (e)
         {
         default:
            RAMFAIL_RETURN(e);
         case RAMFAIL_OK:
            break;
         case RAMFAIL_INPUT:
            return e;
         }
         break;
      }

      case 'l':
      {
         ramfail_status_t e = RAMFAIL_INSANE;

         e = parsemaxsize(&params_arg->ramtestp_maxsize);
         switch (e)
         {
         default:
            RAMFAIL_RETURN(e);
         case RAMFAIL_OK:
            break;
         case RAMFAIL_INPUT:
            return e;
         }
         break;
      }

      case 'S':
      {
         ramfail_status_t e = RAMFAIL_INSANE;

         e = parserngseed(&params_arg->ramtestp_rngseed);
         switch (e)
         {
         default:
            RAMFAIL_RETURN(e);
         case RAMFAIL_OK:
            params_arg->ramtestp_userngseed = 1;
            break;
         case RAMFAIL_INPUT:
            return e;
         }
         break;
      }

      case 'n':
      {
         params_arg->ramtestp_dryrun = 1;
         break;
      }

      case ':':   /* missing argument. */
      case '?':   /* unrecognized option */
         usage(RAMFAIL_INPUT, argc_arg, argv_arg);
         return RAMFAIL_INSANE;

      case 'h':
         usage(RAMFAIL_OK, argc_arg, argv_arg);
         return RAMFAIL_INSANE;
      }
  }

  if (optind < argc_arg)
  {
     fprintf(stderr, "i don't understand what you mean by \"%s\".\n",
           argv_arg[optind]);
     return RAMFAIL_INPUT;
  }

  /* if the test is parallelized, it's meaningless to specify a seed. */
  if (params_arg->ramtestp_userngseed &&
        params_arg->ramtestp_threadcount > 1)
  {
     fprintf(stderr,
           "it's meaningless to specify is seed value for a parallelized "
           "test. either specify --parallelized=1 or omit the --rng-seed "
           "option.\n");
     return RAMFAIL_INPUT;
  }

  return RAMFAIL_OK;
}

ramfail_status_t parseulong(unsigned long *n_arg, const char *str_arg)
{
   unsigned long n = 0;

   RAMFAIL_DISALLOWNULL(n_arg);
   *n_arg = 0;
   RAMFAIL_DISALLOWNULL(str_arg);

   errno = 0;
   n = strtoul(str_arg, NULL, 10);
   if (0 != n || 0 == errno)
   {
      *n_arg = n;
      return RAMFAIL_OK;
   }
   else
      return RAMFAIL_CRT;
}

ramfail_status_t parselong(long *n_arg, const char *str_arg)
{
   long n = 0;

   RAMFAIL_DISALLOWNULL(n_arg);
   *n_arg = 0;
   RAMFAIL_DISALLOWNULL(str_arg);

   errno = 0;
   n = strtol(str_arg, NULL, 10);
   if (0 != n || 0 == errno)
   {
      *n_arg = n;
      return RAMFAIL_OK;
   }
   else
      return RAMFAIL_CRT;
}

ramfail_status_t parsealloccount(size_t *alloccount_arg)
{
   unsigned long n = 0;
   ramfail_status_t e = RAMFAIL_INSANE;

   RAMFAIL_DISALLOWNULL(alloccount_arg);
   *alloccount_arg = 0;

   e = parseulong(&n, optarg);
   switch (e)
   {
   default:
      RAMFAIL_RETURN(e);
   case RAMFAIL_OK:
      *alloccount_arg = n;
      return RAMFAIL_OK;
   case RAMFAIL_CRT:
      fprintf(stderr, "you must specify a numeric argument for the "
            "--allocations (or -a) argument.\n");
      return RAMFAIL_INPUT;
   }
}

ramfail_status_t parsethreadcount(size_t *threadcount_arg)
{
   unsigned long n = 0;
   ramfail_status_t e = RAMFAIL_INSANE;
   size_t cpucount = 0, toomany = 0;

   RAMFAIL_DISALLOWNULL(threadcount_arg);
   *threadcount_arg = 0;

   /* i'll allow up to 3x the number of CPUs in the system. anything more
    * seems nonsensical and could freeze the system. */
   RAMFAIL_RETURN(ramsys_cpucount(&cpucount));
   toomany = 3 * cpucount;

   e = parseulong(&n, optarg);
   switch (e)
   {
   default:
      RAMFAIL_RETURN(e);
   case RAMFAIL_OK:
      if (n > toomany)
      {
         fprintf(stderr, "you cannot specify more than %u parallel "
               "operations.\n", toomany);
         return RAMFAIL_INPUT;
      }
      else if (n < 1)
      {
         fprintf(stderr, "you cannot specify fewer than 1 parallel "
               "operations.\n");
         return RAMFAIL_INPUT;

      }
      else
      {
         *threadcount_arg = n;
         return RAMFAIL_OK;
      }
   case RAMFAIL_CRT:
      fprintf(stderr, "you must specify a numeric argument for the "
            "--parallel (or -p) argument.\n");
      return RAMFAIL_INPUT;
   }
}

ramfail_status_t parsemallocchance(int *mallocchance_arg)
{
   long n = 0;
   ramfail_status_t e = RAMFAIL_INSANE;

   RAMFAIL_DISALLOWNULL(mallocchance_arg);
   *mallocchance_arg = 0;

   e = parselong(&n, optarg);
   switch (e)
   {
   default:
      RAMFAIL_RETURN(e);
   case RAMFAIL_OK:
      if (n >= 100)
      {
         fprintf(stderr,
               "you cannot specify that 100%% or more allocations "
               "be performed with malloc().");
         return RAMFAIL_INPUT;
      }
      else if (n < 0)
      {
         fprintf(stderr, "please specify a valid percentage argument for "
               "the --malloc (or -m) option.");
         return RAMFAIL_INPUT;
      }
      else
      {
         /* it's impossible for this cast to fail, since n cannot be larger
          * than 99. */
         *mallocchance_arg = n;
         return RAMFAIL_OK;
      }
   case RAMFAIL_CRT:
      fprintf(stderr,
            "you must specify a numeric argument representing a percentage "
            "for the --malloc (or -m) argument.\n");
      return RAMFAIL_INPUT;
   }
}

ramfail_status_t parseminsize(size_t *minsize_arg)
{
   unsigned long n = 0;
   ramfail_status_t e = RAMFAIL_INSANE;

   RAMFAIL_DISALLOWNULL(minsize_arg);
   *minsize_arg = 0;

   e = parseulong(&n, optarg);
   switch (e)
   {
   default:
      /* TODO: should i create a new macro for something i know is not
       * RAMFAIL_OK? it would be something akin to rethrowing an
       * exception. */
      RAMFAIL_RETURN(e);
   case RAMFAIL_OK:
      /* the upper and lower bounds for this argument must be policed by
       * the individual tests. */
      *minsize_arg = n;
      return RAMFAIL_OK;
   case RAMFAIL_CRT:
      fprintf(stderr,
            "you must specify a numeric argument for the lower bound of "
            "the allocation size for the --smallest (or -s) argument.\n");
      return RAMFAIL_INPUT;
   }
}

ramfail_status_t parsemaxsize(size_t *maxsize_arg)
{
   unsigned long n = 0;
   ramfail_status_t e = RAMFAIL_INSANE;

   RAMFAIL_DISALLOWNULL(maxsize_arg);
   *maxsize_arg = 0;

   e = parseulong(&n, optarg);
   switch (e)
   {
   default:
      RAMFAIL_RETURN(e);
   case RAMFAIL_OK:
      /* the upper and lower bounds for this argument must be policed by
       * the individual tests. */
      *maxsize_arg = n;
      return RAMFAIL_OK;
   case RAMFAIL_CRT:
      fprintf(stderr,
            "you must specify a numeric argument for the upper bound of "
            "the allocation size for the --largest (or -l) argument.\n");
      return RAMFAIL_INPUT;
   }
}

ramfail_status_t parserngseed(unsigned int *rngseed_arg)
{
   unsigned long n = 0;
   ramfail_status_t e = RAMFAIL_INSANE;

   RAMFAIL_DISALLOWNULL(rngseed_arg);
   *rngseed_arg = 0;

   e = parseulong(&n, optarg);
   switch (e)
   {
   default:
      RAMFAIL_RETURN(e);
   case RAMFAIL_OK:
      /* the upper and lower bounds for this argument must be policed by
       * the individual tests. */
      *rngseed_arg = n;
      return RAMFAIL_OK;
   case RAMFAIL_CRT:
      fprintf(stderr,
            "you must specify a numeric argument for the --rng-seed (or "
            "-S) argument.\n");
      return RAMFAIL_INPUT;
   }
}
