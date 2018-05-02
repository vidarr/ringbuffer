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
#include "../include/buffercache.h"

/******************************************************************************
                               PRIVATE PROTOTYPES
 ******************************************************************************/

static void free_buffer(void* data_buffer, void* arg);

/******************************************************************************
                               PUBLIC FUNCTIONS
 ******************************************************************************/

Ringbuffer* buffercache_create(size_t capacity) {

    return ringbuffer_create(capacity, free_buffer, 0);

}

/*----------------------------------------------------------------------------*/

Buffer* buffercache_get_buffer(Ringbuffer* cache, size_t min_length_bytes) {

    Buffer* db =  0;

    if(0 != cache) {
        db = cache->pop(cache);
    }

    if(0 == db) {
        db = calloc(1, sizeof(Buffer));
    }

    if(db->capacity_bytes < min_length_bytes) {
        if(0 != db->data) {
            free(db->data);
            db->data = 0;
        }
    }

    if(0 == db->data) {
        db->data = calloc(1, sizeof(min_length_bytes) * sizeof(uint8_t));
        db->capacity_bytes = min_length_bytes;
    }

    db->bytes_used = 0;

    return db;

}

/*----------------------------------------------------------------------------*/

bool buffercache_release_buffer(Ringbuffer* cache, Buffer* buffer) {

    if(0 == cache) goto finish;
    if(0 == buffer) goto finish;

    return cache->add(cache, buffer);

finish:

    return false;

}

/*----------------------------------------------------------------------------*/

static void buffer_free(void* buffer, void* additional_arg) {

    // UNUSED(additional_arg);

    if(0 == buffer) goto error;

    Buffer* db = buffer;

    if(0 != db->data) {
        free(db->data);
        db->data = 0;
    }

    free(db);

error:

    return;

}

/*----------------------------------------------------------------------------*/
