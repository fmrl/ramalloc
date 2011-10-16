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

#ifndef RAMALLOC_CAST_H_IS_INCLUDED
#define RAMALLOC_CAST_H_IS_INCLUDED

#include <ramalloc/fail.h>
#include <stddef.h>

/**
 * @addtogroup general
 * @{
 * @file
 * @brief type casting module.
 */

/**
 * @brief calculate the base address of a structure.
 * @details use RAM_CAST_STRUCTBASE() to calculate the base address of a
 *    structure, given the address of one of its member fields.
 * @param StructType
 *    the type of the struct.
 * @param FieldName
 *    the name of the field associated with @e FieldPtr.
 * @param FieldPtr
 *    the address of a field from a struct of type @e StructType.
 * @return this macro evaluates to the base address of the struct
 *    associated with its field at @e FieldPtr.
 */
#define RAM_CAST_STRUCTBASE(StructType, FieldName, FieldPtr) \
   ((StructType *)((char *)(FieldPtr) - offsetof(StructType, FieldName)))

/**
 * @brief convert an @b unsigned @b long to a @b char.
 * @details use ram_cast_ulongtochar() to convert an @b unsigned @b long
 *    value to a @b char. if the value cannot be preserved, this function
 *    notifies the caller.
 * @param to_arg
 *    address of destination value.
 * @param from_arg
 *    the source value (or the value to be converted).
 * @return @c RAM_REPLY_OK - the operation was successful.
 * @return @c RAM_REPLY_RANGEFAIL - the conversion failed because value
 *    could not be preserved.
 * @remark this function returns a @e reply as described in reply.h.
 *    replies not yet documented here may also be passed up through the
 *    callstack. use a reply wrapper from fail.h to trap unexpected
 *    replies.
 */
ram_reply_t ram_cast_ulongtochar(char *to_arg, unsigned long from_arg);

/**
 * @brief convert a @c size_t to an @b int.
 * @details use ram_cast_sizetoint() to convert a @c size_t to an @b int.
 *    if the value cannot be preserved, this function notifies the caller.
 * @param to_arg
 *    address of destination value.
 * @param from_arg
 *    the source value (or the value to be converted).
 * @return @c RAM_REPLY_OK - the operation was successful.
 * @return @c RAM_REPLY_RANGEFAIL - the conversion failed because value
 *    could not be preserved.
 * @remark this function returns a @e reply as described in reply.h.
 *    replies not yet documented here may also be passed up through the
 *    callstack. use a reply wrapper from fail.h to trap unexpected
 *    replies.
 */
ram_reply_t ram_cast_sizetoint(int *to_arg, size_t from_arg);

/**
 * @brief convert a @c size_t to a @b long.
 * @details use ram_cast_sizetolong() to convert a @c size_t to a @b long.
 *    if the value cannot be preserved, this function notifies the caller.
 * @param to_arg
 *    address of destination value.
 * @param from_arg
 *    the source value (or the value to be converted).
 * @return @c RAM_REPLY_OK - the operation was successful.
 * @return @c RAM_REPLY_RANGEFAIL - the conversion failed because value
 *    could not be preserved.
 * @remark this function returns a @e reply as described in reply.h.
 *    replies not yet documented here may also be passed up through the
 *    callstack. use a reply wrapper from fail.h to trap unexpected
 *    replies.
 */
ram_reply_t ram_cast_sizetolong(long *to_arg, size_t from_arg);

/**
 * @brief convert an @b unsigned @b long to a @b long.
 * @details use ram_cast_ulongtolong() to convert an @b unsigned @b long to
 *    a @b long. if the value cannot be preserved, this function notifies
 *    the caller.
 * @param to_arg
 *    address of destination value.
 * @param from_arg
 *    the source value (or the value to be converted).
 * @return @c RAM_REPLY_OK - the operation was successful.
 * @return @c RAM_REPLY_RANGEFAIL - the conversion failed because value
 *    could not be preserved.
 * @remark this function returns a @e reply as described in reply.h.
 *    replies not yet documented here may also be passed up through the
 *    callstack. use a reply wrapper from fail.h to trap unexpected
 *    replies.
 */
ram_reply_t ram_cast_ulongtolong(long *to_arg, unsigned long from_arg);

/**
 * @brief convert a @b long to a @c size_t.
 * @details use ram_cast_longtosize() to convert a @b long to a @c size_t.
 *    if the value cannot be preserved, this function notifies the caller.
 * @param to_arg
 *    address of destination value.
 * @param from_arg
 *    the source value (or the value to be converted).
 * @return @c RAM_REPLY_OK - the operation was successful.
 * @return @c RAM_REPLY_RANGEFAIL - the conversion failed because value
 *    could not be preserved.
 * @remark this function returns a @e reply as described in reply.h.
 *    replies not yet documented here may also be passed up through the
 *    callstack. use a reply wrapper from fail.h to trap unexpected
 *    replies.
 */
ram_reply_t ram_cast_longtosize(size_t *to_arg, long from_arg);

/**
 * @brief convert a @b int to a @c size_t.
 * @details use ram_cast_inttosize() to convert a @b int to a @c size_t.
 *    if the value cannot be preserved, this function notifies the caller.
 * @param to_arg
 *    address of destination value.
 * @param from_arg
 *    the source value (or the value to be converted).
 * @return @c RAM_REPLY_OK - the operation was successful.
 * @return @c RAM_REPLY_RANGEFAIL - the conversion failed because value
 *    could not be preserved.
 * @remark this function returns a @e reply as described in reply.h.
 *    replies not yet documented here may also be passed up through the
 *    callstack. use a reply wrapper from fail.h to trap unexpected
 *    replies.
 */
ram_reply_t ram_cast_inttosize(size_t *to_arg, int from_arg);

/**
 * @brief convert a @c size_t to an @b unsigned @b int.
 * @details use ram_cast_sizetouint() to convert a @c size_t to an
 *    @b unsigned @b int. if the value cannot be preserved, this function
 *    notifies the caller.
 * @param to_arg
 *    address of destination value.
 * @param from_arg
 *    the source value (or the value to be converted).
 * @return @c RAM_REPLY_OK - the operation was successful.
 * @return @c RAM_REPLY_RANGEFAIL - the conversion failed because value
 *    could not be preserved.
 * @remark this function returns a @e reply as described in reply.h.
 *    replies not yet documented here may also be passed up through the
 *    callstack. use a reply wrapper from fail.h to trap unexpected
 *    replies.
 */
ram_reply_t ram_cast_sizetouint(unsigned int *to_arg, size_t from_arg);

/**
 * @brief convert an @b unsigned @b int to an @b int.
 * @details use ram_cast_ulongtouint() to convert an @b unsigned @b int to
 *    an @b int. if the value cannot be preserved, this function notifies
 *    the caller.
 * @param to_arg
 *    address of destination value.
 * @param from_arg
 *    the source value (or the value to be converted).
 * @return @c RAM_REPLY_OK - the operation was successful.
 * @return @c RAM_REPLY_RANGEFAIL - the conversion failed because value
 *    could not be preserved.
 * @remark this function returns a @e reply as described in reply.h.
 *    replies not yet documented here may also be passed up through the
 *    callstack. use a reply wrapper from fail.h to trap unexpected
 *    replies.
 */
ram_reply_t ram_cast_ulongtouint(unsigned int *to_arg,
      unsigned long from_arg);

/**
 * @brief convert an @b unsigned @b long to an @b int.
 * @details use ram_cast_ulongtouint() to convert an @b unsigned @b int to
 *    an @b int. if the value cannot be preserved, this function notifies
 *    the caller.
 * @param to_arg
 *    address of destination value.
 * @param from_arg
 *    the source value (or the value to be converted).
 * @return @c RAM_REPLY_OK - the operation was successful.
 * @return @c RAM_REPLY_RANGEFAIL - the conversion failed because value
 *    could not be preserved.
 * @remark this function returns a @e reply as described in reply.h.
 *    replies not yet documented here may also be passed up through the
 *    callstack. use a reply wrapper from fail.h to trap unexpected
 *    replies.
 */
ram_reply_t ram_cast_ulongtouchar(unsigned char *to_arg,
      unsigned long from_arg);

/**
 * @brief convert a @b long to a @b char.
 * @details use ram_cast_longtochar() to convert a @b long to a @b char.
 *    if the value cannot be preserved, this function notifies the caller.
 * @param to_arg
 *    address of destination value.
 * @param from_arg
 *    the source value (or the value to be converted).
 * @return @c RAM_REPLY_OK - the operation was successful.
 * @return @c RAM_REPLY_RANGEFAIL - the conversion failed because value
 *    could not be preserved.
 * @remark this function returns a @e reply as described in reply.h.
 *    replies not yet documented here may also be passed up through the
 *    callstack. use a reply wrapper from fail.h to trap unexpected
 *    replies.
 */
ram_reply_t ram_cast_longtochar(char *to_arg, long from_arg);

/**
 * @ingroup test
 * @brief test the @e cast module.
 * @details use ram_cast_longtochar() to convert a @b long to a @b char.
 *    if the value cannot be preserved, this function notifies the caller.
 * @return @c RAM_REPLY_OK - the test was successful.
 * @return @c RAM_REPLY_INSANE - the test failed.
 * @remark this function returns a @e reply as described in reply.h.
 *    replies not yet documented here may also be passed up through the
 *    callstack. use a reply wrapper from fail.h to trap unexpected
 *    replies.
 * @remark this function is intended to be used by regression test code.
 *    please ensure that your linker is configured to strip unused code
 *    if you wish to conserve space in the final executable.
 */
ram_reply_t ram_cast_test();

#endif /* RAMALLOC_CAST_H_IS_INCLUDED */

/**
 * @}
 */
