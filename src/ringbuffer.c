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
 *                         PRIVATE DATA STRUCTURES
 ******************************************************************************/

typedef struct Entry {
    void* item;
    struct Entry* next;
} Entry;

/*----------------------------------------------------------------------------*/

typedef struct InternalRingbuffer {

    Ringbuffer public;

    Entry* next_entry_to_read;
    Entry* next_entry_to_write;
    size_t max_num_items;

    void (*free_item)(void* item, void* additional_arg);
    void* free_item_additional_arg;

} InternalRingbuffer;

/******************************************************************************
                                PUBLIC FUNCTIONS
 ******************************************************************************/

Ringbuffer* ringbuffer_create(
        size_t capacity,
        void (*free_item)(void* item, void* additional_arg),
        void* free_item_additional_arg) {

    if(0 >= capacity) {
        goto error;
    }

    Entry* list_start = calloc(1, sizeof(Entry));
    Entry* next = list_start;

    for(size_t i = 1; i < capacity; ++i) {

        next->next = calloc(1, sizeof(Entry));
        next = next->next;

    }

    next->next = list_start;

    InternalRingbuffer* buffer = calloc(1, sizeof(InternalRingbuffer));

    *buffer = (InternalRingbuffer) {
        .next_entry_to_write = list_start,
        .next_entry_to_read = list_start,
        .max_num_items = capacity,
        .free_item = free_item,
        .free_item_additional_arg = free_item_additional_arg,
    };

    buffer->public = (Ringbuffer) {
        .capacity = capacity_func,
        .add = add_func,
        .pop = pop_func,
        .free = free_func,
    };


    return (Ringbuffer*)buffer;

error:

    return 0;
}

/******************************************************************************
  PRIVATE FUNCTIONS
 ******************************************************************************/


static size_t capacity_func(Ringbuffer* self) {

    if(0 == self) goto error;

    InternalRingbuffer* internal = (InternalRingbuffer*) self;

    return internal->max_num_items;

error:

    return 0;

}

/*----------------------------------------------------------------------------*/

static bool add_func(Ringbuffer* self, void* item) {

    if(0 == self) goto error;

    InternalRingbuffer* internal = (InternalRingbuffer*) self;

    Entry* read = internal->next_entry_to_read;
    Entry* write = internal->next_entry_to_write;

    if((read == write) && (0 != read->next->item)) {
        internal->next_entry_to_read = read->next;
    }

    if((0 != write->item) && (0 != internal->free_item)) {
        internal->free_item(
                write->item,
                internal->free_item_additional_arg);
    }

    write->item = item;
    internal->next_entry_to_write = write->next;

    return true;

error:

    return false;

}

/*----------------------------------------------------------------------------*/

static void* pop_func(Ringbuffer* self) {

    if(0 == self) goto error;

    InternalRingbuffer* internal = (InternalRingbuffer*) self;

    Entry* read = internal->next_entry_to_read;

    if(0 == read->item) {
        return 0;
    }

    void* retval = read->item;
    read->item = 0;
    internal->next_entry_to_read = read->next;

    return retval;

error:

    return 0;

}

/*----------------------------------------------------------------------------*/

static Ringbuffer* free_func(Ringbuffer* self) {

    if(0 == self) goto error;

    InternalRingbuffer* internal = (InternalRingbuffer*) self;

    Entry* current = internal->next_entry_to_write;
    Entry* start = current;

    do {

        Entry* next = current->next;

        if((0 != current->item) && (internal->free_item)) {

            internal->free_item(
                    current->item,
                    internal->free_item_additional_arg);

        }

        free(current);
        current = next;

    } while(current != start);

    free(self);
    self = 0;

error:

    return self;

}

/*----------------------------------------------------------------------------*/


