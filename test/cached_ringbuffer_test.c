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
/*----------------------------------------------------------------------------*/


#include "test_helper.h"
#include "../src/ringbuffer.c"
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

/*----------------------------------------------------------------------------*/

static void count_free(void* item, void* additional_arg) {

    if(0 == additional_arg) goto error;

    size_t* counter = additional_arg;
    ++counter;

error:

    return;

}

/*----------------------------------------------------------------------------
 *                             Cached ringbuffer
 *----------------------------------------------------------------------------*/

void cache_free(void* item, void* cache) {

    if(0 == cache) goto finish;
    if(0 == item) goto finish;

    Ringbuffer* ringbuffer_cache = cache;

    if(! ringbuffer_cache->add(ringbuffer_cache, item)) {

        fprintf(stderr, "Could not enqueue item into cache");

    }

finish:

    do{}while(0);

}

/*----------------------------------------------------------------------------
                                  ACTUAL TESTS
  ----------------------------------------------------------------------------*/

void test_cached_ringbuffer_create() {

    Ringbuffer* buffer = 0;

    buffer = ringbuffer_create(1, 0, 0);
    assert(buffer);
    assert(capacity_func == buffer->capacity);
    assert(add_func == buffer->add);
    assert(pop_func == buffer->pop);
    assert(free_func == buffer->free);

    buffer = buffer->free(buffer);

    fprintf(stdout, "cached ringbuffer_create OK\n");

}

/*----------------------------------------------------------------------------
 *                          More sophisiticated cache
 *----------------------------------------------------------------------------*/

/**
 * A data buffer to hold arbitrary data
 */
typedef struct {

    size_t capacity_bytes;
    size_t bytes_used;
    uint8_t* data;

} DataBuffer;

/*----------------------------------------------------------------------------*/

DataBuffer* get_data_buffer(Ringbuffer* cache, size_t min_length_bytes) {

    DataBuffer* db =  0;

    if(0 != cache) {
        db = cache->pop(cache);
    }

    if(0 == db) {
        db = calloc(1, sizeof(DataBuffer));
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

void free_data_buffer(void* data_buffer, void* arg) {

    if(0 != arg) {
        size_t* counter = arg;
        *counter = 1 + *counter;
    }

    if(0 == data_buffer) goto error;
    DataBuffer* db = data_buffer;

    if(0 != db->data) {
        free(db->data);
        db->data = 0;
    }

    free(db);

error:

    return;

}

/*----------------------------------------------------------------------------*/

void test_cached_free() {

    size_t frees_count = 0;

    Ringbuffer* cache =
        ringbuffer_create(50, free_data_buffer, &frees_count);

    Ringbuffer* buffer = ringbuffer_create(21, cache_free, cache);

    size_t i = 0;

    while(10 > frees_count) {
        /* just some abritrary data that has been allocated dynamically */
        ++i;
        /* We want to create really new data buffers, thus cache == 0 ... */
        DataBuffer* db = get_data_buffer(0, 13);
        buffer->add(buffer, db);
    };

    size_t initial_count = frees_count;

    /* Shift some elements out of cache into ringbuffer */
    for(i = 0; i < 3 * buffer->capacity(buffer); ++i) {
        DataBuffer* item = get_data_buffer(cache, i);
        assert(item);
        buffer->add(buffer, item);
    }

    assert(initial_count == frees_count);
    assert(0 == buffer->free(buffer));

    cache = cache->free(cache);

    fprintf(stdout, "free() OK: %zu\n", frees_count);

}

/*----------------------------------------------------------------------------*/
int main(int argc, char** argv) {

    /* The interface tests */
    size_t free_count = 0;
    Ringbuffer* cache = ringbuffer_create(31, count_free, &free_count);
    free_item = cache_free;
    free_item_additional_arg = cache;

    test_ringbuffer_create();
    test_capacity();
    test_add();
    test_pop();
    cache->free(cache);
    cache = 0;

    /* Caching tests */
    test_cached_ringbuffer_create();
    test_cached_free();

}

/*----------------------------------------------------------------------------*/
