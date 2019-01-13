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
/**
 * This file provides a ringbuffer. See the Ringbuffer struct.
 */
#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__
/*----------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdbool.h>

/*----------------------------------------------------------------------------*/

/**
 * A ringbuffer is a first-in first-out queue with a fixed capacity.
 * It handles arbitrary pointers.
 * If an attempt is made to add a pointer to a filled ringbuffer, the
 * oldest pointer in the ringbuffer is overwritten.
 */
typedef struct Ringbuffer {

    /**
     * Get the number of elements this ringbuffer might hold before overwriting elements.
     */
    size_t        (*capacity) (struct Ringbuffer* self);

    /**
     * add an element to this ringbuffer
     * @return true on success, false in case of failure
     */
    bool          (*add)      (struct Ringbuffer* self, void* item);

    /**
     * Retrieve the oldest element from the ringbuffer.
     * The element is removed from the ringbuffer.
     * @return the oldest element from the ringbuffer.
     */
    void*         (*pop)      (struct Ringbuffer* self);

    /**
     * Free this ringbuffer and all elements contained within.
     * @return 0 on success or self in case of error.
     */
    struct Ringbuffer* (*free)  (struct Ringbuffer* self);

} Ringbuffer;

/*----------------------------------------------------------------------------*/

/**
 * Create a new Ringbuffer.
 * @param capacity number of elements this ringbuffer can hold before overwriting elements.
 * @param free_item function to free elements. If 0, elements that are overwritten wont be freed.
 * @param free_item_additional_arg arbitrary pointer handed over to free_item
 */
Ringbuffer* ringbuffer_create(
        size_t capacity,
        void (*free_item)(void* item, void* additional_arg),
        void* free_item_additional_arg);

/*----------------------------------------------------------------------------*/

#endif
