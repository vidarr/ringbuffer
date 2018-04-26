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

#include "../include/ringbuffer.h"

/******************************************************************************
                               PRIVATE PROTOTYPES
 ******************************************************************************/

static size_t capacity_func(Ringbuffer* self);

static bool add_func(Ringbuffer* self, void* item);

static void* pop_func(Ringbuffer* self);

static Ringbuffer* free_func(Ringbuffer* self);


/******************************************************************************
                                PUBLIC FUNCTIONS
 ******************************************************************************/

Ringbuffer* ringbuffer_create(
        size_t capacity,
        void (*free_item)(void* item, void* additional_arg),
        void* free_item_additional_arg) {

    assert(0 < capacity);

    if(0 >= capacity) {
        goto error;
    }

    Ringbuffer* buffer = calloc(1, sizeof(Ringbuffer));

    *buffer = (Ringbuffer) {
        .capacity = capacity_func,
        .add = add_func,
        .pop = pop_func,
        .free = free_func

    };

    return buffer;

error:

    return 0;
}

/******************************************************************************
                               PRIVATE FUNCTIONS
 ******************************************************************************/


static size_t capacity_func(Ringbuffer* self) {

    return 0;

}

/*----------------------------------------------------------------------------*/

static bool add_func(Ringbuffer* self, void* item) {

    return false;

}

/*----------------------------------------------------------------------------*/

static void* pop_func(Ringbuffer* self) {

    return 0;

}

/*----------------------------------------------------------------------------*/

static Ringbuffer* free_func(Ringbuffer* self) {

    return self;

}

/*----------------------------------------------------------------------------*/


