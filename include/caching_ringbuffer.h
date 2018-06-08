/*
 * (C) 2018 Michael J. Beer
 * All rights reserved.
 *
 * Redistribution  and use in source and binary forms, with or with‐
 * out modification, are permitted provided that the following  con‐
 * ditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above  copy‐
 * right  notice,  this  list  of  conditions and the following dis‐
 * claimer in the documentation and/or other materials provided with
 * the distribution.
 *
 * 3.  Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote  products  derived
 * from this software without specific prior written permission.
 *
 * THIS  SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBU‐
 * TORS "AS IS" AND ANY EXPRESS OR  IMPLIED  WARRANTIES,  INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE  ARE  DISCLAIMED.  IN  NO  EVENT
 * SHALL  THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DI‐
 * RECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR  CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS IN‐
 * TERRUPTION)  HOWEVER  CAUSED  AND  ON  ANY  THEORY  OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING  NEGLI‐
 * GENCE  OR  OTHERWISE)  ARISING  IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __CACHING_RINGBUFFER_H__
#define __CACHING_RINGBUFFER_H__
/*----------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

/*----------------------------------------------------------------------------*/

/**
 * Create a new caching Ringbuffer.
 * @param capacity number of elements this ringbuffer can hold before overwriting elements.
 * @param free_item function to free elements. If 0, elements that are overwritten wont be freed.
 * @param free_item_additional_arg arbitrary pointer handed over to free_item
 * @see ringbuffer_create
 */
Ringbuffer* caching_ringbuffer_create(
        size_t capacity,
        void (*free_item)(void* item, void* additional_arg),
        void* free_item_additional_arg);

/*----------------------------------------------------------------------------*/

void* caching_ringbuffer_get_cached(Ringbuffer* crb);

/*----------------------------------------------------------------------------*/

bool caching_ringbuffer_release(Ringbuffer* crb, void* item);

/*----------------------------------------------------------------------------*/

#endif
