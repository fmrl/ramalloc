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

/**
 * @file
 * @brief failure management module.
 */

#ifndef RAMALLOC_FAIL_H_IS_INCLUDED
#define RAMALLOC_FAIL_H_IS_INCLUDED

#include <ramalloc/opt.h>
#include <ramalloc/meta.h>
#include <ramalloc/annotate.h>
#include <ramalloc/reply.h>
#include <assert.h>
#include <stdlib.h>

/**
 * @brief signature of the unanticipated reply reporter.
 * @details a reporter performs a client-defined action when a reply wrapper
 *    detects an unanticipated reply.
 * @param code_arg the reply code encountered.
 * @param expr_arg a string containing the expression being evaluated.
 *    this is usually a preprocessor-generated string.
 * @param funcn_arg a string containing the name of the function where
 *    @e expr_arg can be found.
 * @param filen_arg a string containing the name of the file where
 *    @e expr_arg can be found.
 * @param lineno_arg the line number where expr_arg can be found in file
 *    @e filen_arg.
 * @see ram_fail_setreporter()
 */
typedef void (*ram_fail_reporter_t)(ram_reply_t code_arg,
      const char *expr_arg, const char *funcn_arg, const char *filen_arg,
      int lineno_arg);

/**
 * @brief reply if precondition is not met.
 * @details use RAM_FAIL_EXPECT() to reply if a given boolean expression
 *    evaluates to @e false.
 * @param Reply how to reply, should @e Condition be @e false. reply @c
 *    RAM_REPLY_OK is disallowed.
 * @param Condition a boolean expression that determines whether a function
 *    should reply early or not.
 * @remark if @e Condition is @e true, then this macro will cause the
 *    function to immediately return a reply to the caller. this means that
 *    the contextual function must have declared its return type as
 *    ram_reply_t for this macro to function properly.
 * @remark if RAMOPT_UNSUPPORTED_OVERCONFIDENT expands to @e true, then this
 *    macro does nothing.
 * @see ram_reply_t
 */
#define RAM_FAIL_EXPECT(Reply, Condition) \
   do \
   { \
      assert(RAM_REPLY_OK != (Reply)); \
      RAMMETA_IFTHEN(!(Condition), \
            ram_fail_report((Reply), #Condition, NULL, __FILE__, \
                  __LINE__); return (Reply)); \
   } \
   while (0)

/**
 * @brief disallow argument value zero.
 * @details use RAM_FAIL_NOTZERO() to prevent a function argument from
 *    having the value 0.
 * @param Value the value to test. if 0, then the contextual function
 *    returns immediately with reply @e RAM_REPLY_DISALLOWED.
 * @remark if the disallowed value is encountered, this macro will cause the
 *    function to immediately return a reply to the caller. this means that
 *    the contextual function must have declared its return type as
 *    ram_reply_t for this macro to function properly.
 * @remark if RAMOPT_UNSUPPORTED_OVERCONFIDENT expands to @e true, then this
 *    macro has no effect.
 * @see ram_reply_t
 */
#if RAMOPT_UNSUPPORTED_OVERCONFIDENT
#  define RAM_FAIL_NOTZERO(Value) RAMANNOTATE_UNUSEDARG(Value)
#else
#  define RAM_FAIL_NOTZERO(Value) \
      RAM_FAIL_EXPECT(RAM_REPLY_DISALLOWED, 0 != (Value))
#endif

/**
 * @brief disallow argument value @c NULL.
 * @details use RAM_FAIL_NOTNULL() to prevent a function argument from
 *    having the value @c NULL.
 * @param Value the value to test. if 0, then the contextual function
 *    returns immediately with reply @e RAM_REPLY_DISALLOWED.
 * @remark if the disallowed value is encountered, this macro will cause the
 *    function to immediately return a reply to the caller. this means that
 *    the contextual function must have declared its return type as
 *    ram_reply_t for this macro to function properly.
 * @remark if RAMOPT_UNSUPPORTED_OVERCONFIDENT expands to @e true, then this
 *    macro has no effect.
 * @see ram_reply_t
 */
#if RAMOPT_UNSUPPORTED_OVERCONFIDENT
#  define RAM_FAIL_NOTNULL(Value) RAMANNOTATE_UNUSEDARG(Value)
#else
#  define RAM_FAIL_NOTNULL(Value) \
      RAM_FAIL_EXPECT(RAM_REPLY_DISALLOWED, NULL != (Value))
#endif

/**
 * @internal
 * @brief support macro for RAM_FAIL_TRAP().
 * @param Reply the reply expression to check.
 * @param ReplyCache a prefix to use to name a variable used to cache the
 *    value of @e Reply.
 * @see RAM_FAIL_TRAP().
 */
#if RAMOPT_UNSUPPORTED_OVERCONFIDENT
#  define RAM_FAIL_TRAP2(Reply, ReplyCache) ((void)(Reply))
#else
#  define RAM_FAIL_TRAP2(Reply, ReplyCache) \
         do \
         { \
            const ram_reply_t ReplyCache = (Reply); \
            RAMMETA_IFTHEN(RAM_REPLY_OK != ReplyCache, \
                  ram_fail_report(ReplyCache, #Reply, NULL, __FILE__, \
                        __LINE__); return ReplyCache); \
         } \
         while (0)
#endif

/**
 * @brief the default reply wrapper.
 * @details this macro is a reply wrapper that designates all replies
 *    returned by a function as unanticipated, with the exception of @c
 *    RAM_REPLY_OK.
 * @param Reply an expression that evaluates to a reply.
 * @remark if you are unsure of which reply wrapper to use,
 *    RAM_FAIL_TRAP() is a good default.
 * @remark if an unanticipated reply is encountered, this macro will cause the
 *    function to return that reply to the caller. this means that the
 *    contextual function must have declared its return type as ram_reply_t
 *    for this macro to function properly.
 * @remark if RAMOPT_UNSUPPORTED_OVERCONFIDENT expands to @e true, then this
 *    macro simply evaluates @e Reply.
 * @see ram_reply_t
 * @see reply.h
 */
#if RAMOPT_UNSUPPORTED_OVERCONFIDENT
#  define RAM_FAIL_TRAP(Reply) (Reply)
#else
#define RAM_FAIL_TRAP(Reply) \
         RAM_FAIL_TRAP2((Reply), \
               RAMMETA_GENERATENAME(RAM_REPLY_TRAP_replycache))
#endif

/**
 * @brief denotes unreachable code.
 * @details use RAM_FAIL_UNREACHABLE() to indicate a part of the code that
 *    should never be executed (e.g. following a call to a function that
 *    never returns). if this is ever discovered to not be the case, the
 *    process will panic.
 * @remark this macro expands to code that returns @c RAM_REPLY_INSANE to
 *    prevent unnecessary warnings from being generated. this means that
 *    the contextual function must have declared its return type as
 *    ram_reply_t for this macro to function properly.
 */
#define RAM_FAIL_UNREACHABLE() \
   do \
   { \
      ram_fail_panic("unreachable code"); \
      return RAM_REPLY_INSANE; \
   } \
   while (0);

/**
 * @brief panics if reply is not okay.
 * @details like RAM_FAIL_TRAP(), this macro is a reply wrapper that
 *    designates all replies returned by a function as unanticipated, with the
 *    exception of @c RAM_REPLY_OK. unlike RAM_FAIL_TRAP(),
 *    RAM_FAIL_PANIC() will terminate the process.
 * @remark this macro expands to code that returns @c RAM_REPLY_INSANE to
 *    prevent unnecessary warnings from being generated. this means that
 *    the contextual function must have declared its return type as
 *    ram_reply_t for this macro to function properly.
 */
#if RAMOPT_UNSUPPORTED_OVERCONFIDENT
#  define RAM_FAIL_PANIC(Reply) ((void)(Reply))
#else
#define RAM_FAIL_PANIC(Reply) \
         RAMMETA_IFTHEN(RAM_REPLY_OK != (Reply), \
               ram_fail_panic("omgwtfbbq!"); RAM_FAIL_UNREACHABLE(); )
#endif

/**
 * @brief terminate the process due to an irrecoverable failure.
 * @details ram_fail_panic() prints a message to @b stderr and terminates
 *    the process by calling @c abort().
 * @param why_arg a message for the user explaining why the process stopped.
 * @remarks ram_fail_panic() does not return. follow up with
 *    RAM_FAIL_UNREACHABLE() afterwards to ensure that you don't generate
 *    unnecessary warnings in a function that returns a ram_reply_t.
 */
void ram_fail_panic(const char *why_arg);

/**
 * @brief change the reporting callback.
 * @details call ram_fail_setreporter() to change the unanticipated reply
 *    reporting callback.
 * @param reporter_arg the address of the reporting callback.
 * @see ram_fail_reporter_t
 */
void ram_fail_setreporter(ram_fail_reporter_t reporter_arg);

/**
 * @internal
 * @brief report an unanticipated reply.
 * @details ram_fail_report() will invoke the unanticipated reply reporting
 *    callback, either as specified by ram_fail_setreporter() or a default
 *    implementation.
 * @param reply_arg an unanticipated reply to report.
 * @param expr_arg a string representation of the expression that produced
 *    the unanticipated reply.
 * @param funcn_arg the name of the function that received the unanticipated
 *    reply.
 * @param filen_arg the name of the source file where the expression can be
 *    found.
 * @param lineno_arg the line number where the expression can be found.
 * @todo currently, @e funcn_arg isn't actually being used anywhere.
 */
void ram_fail_report(ram_reply_t reply_arg, const char *expr_arg,
      const char *funcn_arg, const char *filen_arg, int lineno_arg);

/**
 * @brief accumulate replies.
 * @details ram_fail_accumulate() is used to capture the first reply
 *    that is not RAM_REPLY_OK in a series of calls with a common reply_arg
 *    argument. this is useful because usually, in a series of operations,
 *    the first failure is the one that is interesting.
 * @param reply_arg the address of the reply accumulator. the reply must
 *    be initialized to RAM_REPLY_OK. if it ceases to remain this value in
 *    the course of a series of calls to ram_fail_accumulate(), then it will
 *    retain it's value.
 * @param newreply_arg a new reply for the accumulator.
 * @remarks it's always better to return from a function if an unanticipated
 *    reply is encountered. sometimes, this isn't possible and
 *    ram_fail_accumulate() is intended for use in those situations.
 * @return a reply as described in reply.h.
 */
ram_reply_t ram_fail_accumulate(ram_reply_t *reply_arg,
      ram_reply_t newreply_arg);

#endif /* RAMALLOC_FAIL_H_IS_INCLUDED */
