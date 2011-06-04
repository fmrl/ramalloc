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
 * @file reply.h
 * @brief reply definition header.
 * @par overview
 *    @b replies are an attempt to provide a thorough, yet easy-to-use error
 *    reporting mechanism in standard C. if something could fail during the
 *    course of a function's evaluation, that function must return a
 *    @e reply to the caller. the caller then uses a @e reply filter to
 *    distinguish between @e anticipated @e replies and @e unanticipated
 *    @e replies and takes appropriate action for each.
 * @par anticipated replies
 *    an @b anticipated @b reply is a reply for which the programmer is
 *    aware is a possibility and has written the necessary code in a calling
 *    function to act accordingly. only @c RAM_REPLY_OK is considered
 *    anticipated by default, since programmers tend to prioritize
 *    consideration of the success case. an example of a reply that
 *    could be considered anticipated is @c RAM_REPLY_INPUTFAIL, which
 *    is intended to indicate when a user provides an invalid value
 *    through an interface. if the programmer has tested for
 *    @c RAM_REPLY_INPUTFAIL and written code that reports the problem to
 *    the user, then this reply can be considered @e anticipated; otherwise,
 *    it must remain treated as @e unanticipated.
 * @par unanticipated replies
 *    an @b unanticipated @b reply is a reply for which the programmer has not
 *    (yet) considered a possibility, or a reply for which the programmer
 *    cannot reasonably write code to recover from. a @e reply @e filter
 *    determines whether a reply is unanticipated and dispatches the reply
 *    to a @e reporter callback.
 * @par reply filters
 *    a @b reply @b filter detects and passes unanticipated replies to
 *    a @e reporter callback, which usually ends the process after notifying
 *    the operator of the failure. a programmer should use RAM_FAIL_TRAP()
 *    as the default reply filter and specialize as needed.
 *    @par
 *    often, a programmer will have identified certain replies that should
 *    not cause the application to fail and would benefit from an alternate
 *    approach. in this case, the programmer uses a @b switch statement to
 *    implement the reply filter:
 * @code
 * {
 *    ram_reply_t reply = RAM_FAIL_INSANE;
 *
 * retry:
 *    reply = foo();
 *    switch (reply)
 *    {
 *    default:
 *       RAM_FAIL_TRAP(reply);
 *    case RAM_REPLY_INPUTFAIL:
 *       fprintf(stderr, "try again.\n");
 *       goto retry;
 *    case RAM_REPLY_OK:
 *       break;
 *    }
 * }
 * @endcode
 *    in the preceding example, the programmer has communicated that she
 *    has considered two possibilities when calling @c foo(): both
 *    @c RAMFAIL_REPLY_OK and @c RAM_REPLY_INPUTFAIL are designated
 *    @e anticipated @e replies and have unique logic handling their
 *    occurrence. the default branch is used to inherit behavior from
 *    RAM_FAIL_TRAP() for all remaining replies, which will be treated as
 *    unanticipated.
 * @see ram_reply_t
 * @see fail.h
 * @see RAM_FAIL_TRAP()
 * @see ram_fail_reporter_t
 */

#ifndef RAMALLOC_REPLY_H_IS_INCLUDED
#define RAMALLOC_REPLY_H_IS_INCLUDED

/**
 * @brief replies.
 */
typedef enum ram_reply
{
   /** indicates that everything is fine. this is almost always considered
    * an anticipated reply. */
   RAM_REPLY_OK = 0,
   /** indicates that the logic is probably bad. review assumptions the code
    * makes. */
   RAM_REPLY_INSANE,
   /** indicates a C runtime library related failure; check errno. */
   RAM_REPLY_CRTFAIL,
   /** indicates that a foreign API returned a failure code. */
   RAM_REPLY_APIFAIL,
   /** indicates a specific, disallowed value was provided as a function
    * argument. */
   RAM_REPLY_DISALLOWED,
   /** indicates that a value is out of its permitted range. */
   RAM_REPLY_RANGEFAIL,
   /** indicates a resource (e.g. file) related problem. */
   RAM_REPLY_RESOURCEFAIL,
   /** indicates that a search failed. */
   RAM_REPLY_NOTFOUND,
   /** indicates that a specific action or request is unsupported. */
   RAM_REPLY_UNSUPPORTED,
   /** indicates that a complex state is inconsistent. */
   RAM_REPLY_INCONSISTENT,
   /** indicates that an action or request needs to be performed again. */
   RAM_REPLY_AGAIN,
   /** indicates that a data structure failed a runtime check. */
   RAM_REPLY_CORRUPT,
   /** indicates that an accumulator or collection exceeded its maximum. */
   RAM_REPLY_UNDERFLOW,
   /** indicates that an accumulator or collection exceeded its minimum. */
   RAM_REPLY_OVERFLOW,
   /** indicates a user input related problem. */
   RAM_REPLY_INPUTFAIL,
} ram_reply_t;

#endif /* RAMALLOC_REPLY_H_IS_INCLUDED */
