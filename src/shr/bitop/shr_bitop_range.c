/*! \file shr_bitop.c
 *
 * Bit array operations.
 */
/*
 * Copyright: (c) 2018 Broadcom. All Rights Reserved. "Broadcom" refers to 
 * Broadcom Limited and/or its subsidiaries.
 * 
 * Broadcom Switch Software License
 * 
 * This license governs the use of the accompanying Broadcom software. Your 
 * use of the software indicates your acceptance of the terms and conditions 
 * of this license. If you do not agree to the terms and conditions of this 
 * license, do not use the software.
 * 1. Definitions
 *    "Licensor" means any person or entity that distributes its Work.
 *    "Software" means the original work of authorship made available under 
 *    this license.
 *    "Work" means the Software and any additions to or derivative works of 
 *    the Software that are made available under this license.
 *    The terms "reproduce," "reproduction," "derivative works," and 
 *    "distribution" have the meaning as provided under U.S. copyright law.
 *    Works, including the Software, are "made available" under this license 
 *    by including in or with the Work either (a) a copyright notice 
 *    referencing the applicability of this license to the Work, or (b) a copy 
 *    of this license.
 * 2. Grant of Copyright License
 *    Subject to the terms and conditions of this license, each Licensor 
 *    grants to you a perpetual, worldwide, non-exclusive, and royalty-free 
 *    copyright license to reproduce, prepare derivative works of, publicly 
 *    display, publicly perform, sublicense and distribute its Work and any 
 *    resulting derivative works in any form.
 * 3. Grant of Patent License
 *    Subject to the terms and conditions of this license, each Licensor 
 *    grants to you a perpetual, worldwide, non-exclusive, and royalty-free 
 *    patent license to make, have made, use, offer to sell, sell, import, and 
 *    otherwise transfer its Work, in whole or in part. This patent license 
 *    applies only to the patent claims licensable by Licensor that would be 
 *    infringed by Licensor's Work (or portion thereof) individually and 
 *    excluding any combinations with any other materials or technology.
 *    If you institute patent litigation against any Licensor (including a 
 *    cross-claim or counterclaim in a lawsuit) to enforce any patents that 
 *    you allege are infringed by any Work, then your patent license from such 
 *    Licensor to the Work shall terminate as of the date such litigation is 
 *    filed.
 * 4. Redistribution
 *    You may reproduce or distribute the Work only if (a) you do so under 
 *    this License, (b) you include a complete copy of this License with your 
 *    distribution, and (c) you retain without modification any copyright, 
 *    patent, trademark, or attribution notices that are present in the Work.
 * 5. Derivative Works
 *    You may specify that additional or different terms apply to the use, 
 *    reproduction, and distribution of your derivative works of the Work 
 *    ("Your Terms") only if (a) Your Terms provide that the limitations of 
 *    Section 7 apply to your derivative works, and (b) you identify the 
 *    specific derivative works that are subject to Your Terms. 
 *    Notwithstanding Your Terms, this license (including the redistribution 
 *    requirements in Section 4) will continue to apply to the Work itself.
 * 6. Trademarks
 *    This license does not grant any rights to use any Licensor's or its 
 *    affiliates' names, logos, or trademarks, except as necessary to 
 *    reproduce the notices described in this license.
 * 7. Limitations
 *    Platform. The Work and any derivative works thereof may only be used, or 
 *    intended for use, with a Broadcom switch integrated circuit.
 *    No Reverse Engineering. You will not use the Work to disassemble, 
 *    reverse engineer, decompile, or attempt to ascertain the underlying 
 *    technology of a Broadcom switch integrated circuit.
 * 8. Termination
 *    If you violate any term of this license, then your rights under this 
 *    license (including the license grants of Sections 2 and 3) will 
 *    terminate immediately.
 * 9. Disclaimer of Warranty
 *    THE WORK IS PROVIDED "AS IS" WITHOUT WARRANTIES OR CONDITIONS OF ANY 
 *    KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WARRANTIES OR CONDITIONS OF 
 *    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE OR 
 *    NON-INFRINGEMENT. YOU BEAR THE RISK OF UNDERTAKING ANY ACTIVITIES UNDER 
 *    THIS LICENSE. SOME STATES' CONSUMER LAWS DO NOT ALLOW EXCLUSION OF AN 
 *    IMPLIED WARRANTY, SO THIS DISCLAIMER MAY NOT APPLY TO YOU.
 * 10. Limitation of Liability
 *    EXCEPT AS PROHIBITED BY APPLICABLE LAW, IN NO EVENT AND UNDER NO LEGAL 
 *    THEORY, WHETHER IN TORT (INCLUDING NEGLIGENCE), CONTRACT, OR OTHERWISE 
 *    SHALL ANY LICENSOR BE LIABLE TO YOU FOR DAMAGES, INCLUDING ANY DIRECT, 
 *    INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF 
 *    OR RELATED TO THIS LICENSE, THE USE OR INABILITY TO USE THE WORK 
 *    (INCLUDING BUT NOT LIMITED TO LOSS OF GOODWILL, BUSINESS INTERRUPTION, 
 *    LOST PROFITS OR DATA, COMPUTER FAILURE OR MALFUNCTION, OR ANY OTHER 
 *    COMMERCIAL DAMAGES OR LOSSES), EVEN IF THE LICENSOR HAS BEEN ADVISED OF 
 *    THE POSSIBILITY OF SUCH DAMAGES.
 */

#include <shr/shr_bitop.h>

/*!
 * INTERNAL USE ONLY.
 *
 * Same as SHRi_BITOP_RANGE, but for a single SHR_BITDCL.
 */
#define SHRi_BITOP_RANGE_ONE_BITDCL(_a1,_a2, _offs, _n, _dest, _op)     \
    {                                                                   \
        SHR_BITDCL _mask = ~0;                                          \
        SHR_BITDCL _data;                                               \
                                                                        \
        _mask >>= (SHR_BITWID - (_n));                                  \
        _mask <<=_offs;                                                 \
        _data = ((_a1) _op (_a2)) & _mask;                              \
        *(_dest) &= ~_mask;                                             \
        *(_dest) |= _data;                                              \
    }

/*!
 * \brief Perform bitwise logical operation on bit arrays.
 *
 * INTERNAL USE ONLY.
 *
 * This macro allows code sharing between different types of bitwise
 * logical operations between two bit arrays.
 *
 * \param [in] _a1 First bit array for operation
 * \param [in] _a2 Second bit array for operation
 * \param [in] _offs Offset (in bits) into the arrays
 * \param [in] _n Number of bits to operate on
 * \param [in] _dest Destination bit array
 * \param [in] _op Type operation (applied in between BITDCLs)
 *
 * \return Nothing
 */
#define SHRi_BITOP_RANGE(_a1, _a2, _offs, _n, _dest, _op)               \
    {                                                                   \
        const SHR_BITDCL *_ptr_a1;                                      \
        const SHR_BITDCL *_ptr_a2;                                      \
        SHR_BITDCL *_ptr_dest;                                          \
        int _woffs, _wremain;                                           \
                                                                        \
        _ptr_a1 = (_a1) + ((_offs) / SHR_BITWID);                       \
        _ptr_a2 = (_a2) + ((_offs) / SHR_BITWID);                       \
        _ptr_dest = (_dest) + ((_offs) / SHR_BITWID);                   \
        _woffs = ((_offs) % SHR_BITWID);                                \
                                                                        \
        _wremain = SHR_BITWID - _woffs;                                 \
        if ((_n) <= _wremain) {                                         \
            SHRi_BITOP_RANGE_ONE_BITDCL(*_ptr_a1, *_ptr_a2,             \
                                        _woffs, (_n),                   \
                                        _ptr_dest, _op);                \
            return;                                                     \
        }                                                               \
        SHRi_BITOP_RANGE_ONE_BITDCL(*_ptr_a1, *_ptr_a2,                 \
                                    _woffs, _wremain,                   \
                                    _ptr_dest, _op);                    \
        (_n) -= _wremain;                                               \
        ++_ptr_a1; ++_ptr_a2; ++_ptr_dest;                              \
        while ((_n) >= SHR_BITWID) {                                    \
            *_ptr_dest =                                                \
                (*_ptr_a1) _op (*_ptr_a2);                              \
            (_n) -= SHR_BITWID;                                         \
            ++_ptr_a1; ++_ptr_a2; ++_ptr_dest;                          \
        }                                                               \
        if ((_n) > 0) {                                                 \
            SHRi_BITOP_RANGE_ONE_BITDCL(*_ptr_a1, *_ptr_a2,             \
                                        0, (_n),                        \
                                        _ptr_dest, _op);                \
        }                                                               \
    }

/*!
 * \brief Perform bitwise AND operation on bit arrays.
 *
 * INTERNAL USE ONLY.
 *
 * See \ref SHR_BITAND_RANGE for details.
 */
void
shr_bitop_range_and(const SHR_BITDCL *a1, const SHR_BITDCL *a2,
                    const int offs, int n, SHR_BITDCL *dest)
{
    if (n > 0) {
        SHRi_BITOP_RANGE(a1, a2, offs, n, dest, &);
    }
}

/*!
 * \brief Perform bitwise OR operation on bit arrays.
 *
 * INTERNAL USE ONLY.
 *
 * See \ref SHR_BITOR_RANGE for details.
 */
void
shr_bitop_range_or(const SHR_BITDCL *a1, const SHR_BITDCL *a2,
                   const int offs, int n, SHR_BITDCL *dest)
{
    if (n > 0) {
        SHRi_BITOP_RANGE(a1, a2, offs, n, dest, |);
    }
}

/*!
 * \brief Perform bitwise XOR operation on bit arrays.
 *
 * INTERNAL USE ONLY.
 *
 * See \ref SHR_BITXOR_RANGE for details.
 */
void
shr_bitop_range_xor(const SHR_BITDCL *a1, const SHR_BITDCL *a2,
                    const int offs, int n, SHR_BITDCL *dest)
{
    if (n > 0) {
        SHRi_BITOP_RANGE(a1, a2, offs, n, dest, ^);
    }
}

/*!
 * \brief Clear select bits in a bit array.
 *
 * INTERNAL USE ONLY.
 *
 * See \ref SHR_BITREMOVE_RANGE for details.
 */
void
shr_bitop_range_remove(const SHR_BITDCL *a1, const SHR_BITDCL *a2,
                       const int offs, int n, SHR_BITDCL *dest)
{
    if (n > 0) {
        SHRi_BITOP_RANGE(a1, a2, offs, n, dest, & ~);
    }
}
