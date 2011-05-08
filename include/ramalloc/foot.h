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

#ifndef RAMFOOT_H_IS_INCLUDED
#define RAMFOOT_H_IS_INCLUDED

#include <ramalloc/fail.h>
#include <ramalloc/sig.h>
#include <ramalloc/meta.h>
#include <ramalloc/sys.h>
#include <ramalloc/stdint.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct ramfoot_spec
{
   /* the footer offset is the distance, in bytes, from the page's base address to the footer's address. */
   size_t footer_offset;
   /* the storage offset is the distance, in bytes, from the footer to the footer's storage. this
    * is usually 4 or 8 bytes, depending upon the system architecture and the alignment of the struct
    * stored in the footer. */
   size_t storage_offset;
   /* the writable zone is the amount of the page that can be used to store data. the portion past the
    * writable zone is considered the responsibility of another allocator. this value is only used at
    * initialization and is preserved for diagnostic purposes. */
   size_t writable_zone;
   /* the footer size is the length of the footer, in bytes. it can be calculated automatically, given
    * a structure defining what should be stored, by RAMFOOT_MKSPEC(). */
   size_t footer_size;
   /* the footer alignment is the alignment of the footer. it can be calculated automatically, given
    * a structure defining what should be stored, by RAMFOOT_MKSPEC(). this value is only used at 
    * initialization and is preserved for diagnostic purposes. */
   size_t footer_alignment;
   /* the master signature is used to determine what value footer signatures should be initialized with
    * and is used to compare against during access. */
   ramsig_signature_t master_signature;
} ramfoot_spec_t;

ramfail_status_t ramfoot_mkspec(ramfoot_spec_t *spec_arg, size_t writezn_arg, 
   size_t footsz_arg, size_t storalign_arg, size_t storofs_arg, const char *sig_arg);
ramfail_status_t ramfoot_mkfooter(void **storage_arg, const ramfoot_spec_t *spec_arg, void *page_arg);
ramfail_status_t ramfoot_getstorage(void **result_arg,
      const ramfoot_spec_t *spec_arg, void *ptr_arg);

#define RAMFOOT_MKSPEC2(Spec, Type, WriteZone, Signature, FooterTag, TmpVar) \
   do { \
      struct FooterTag \
      { \
         ramsig_signature_t ft_signature; \
         Type ft_storage; \
      }; \
      RAMFAIL_RETURN(ramfoot_mkspec((Spec), (WriteZone), \
         sizeof(struct FooterTag), RAMSYS_ALIGNOF(struct FooterTag), \
         offsetof(struct FooterTag, ft_storage), Signature)); \
   } while (0)

#define RAMFOOT_MKSPEC(Spec, Type, WriteZone, Signature) \
   RAMFOOT_MKSPEC2(Spec, Type, WriteZone, Signature, \
      RAMMETA_GENERATENAME(RAMFOOT_MKSPEC_footer), RAMMETA_GENERATENAME(RAMFOOT_MKSPEC_unused))

#endif /* RAMFOOT_H_IS_INCLUDED */
