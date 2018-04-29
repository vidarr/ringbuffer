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

/*----------------------------------------------------------------------------
                               Cached ringbuffer
  ----------------------------------------------------------------------------*/

/**
 * This is the global cache.
 */
static Ringbuffer* cache = 0;

static size_t free_count = 0;

/*----------------------------------------------------------------------------*/

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

/*----------------------------------------------------------------------------*/

void count_free(void* string_pointer, void* count) {

    size_t* c = (size_t*) count;
    *c = *c + 1;

}

/*----------------------------------------------------------------------------*/

void count_string_free(void* string_pointer, void* count) {

    size_t* c = (size_t*) count;
    *c = *c + 1;

    char* string = string_pointer;
    free(string);

}

/*----------------------------------------------------------------------------*/

void test_cached_free() {

    Ringbuffer* buffer = ringbuffer_create(21, cache_free, cache);

    size_t i = 0;

    while(10 > free_count) {
        /* just some abritrary data that has been allocated dynamically */
        char* string = calloc(1, 100);
        buffer->add(buffer, string);
    };

    size_t initial_count = free_count;

    /* Shift some elements out of cache into ringbuffer */
    for(i = 0; i < 3 * buffer->capacity(buffer); ++i) {
        size_t* item = cache->pop(cache);
        assert(item);
        buffer->add(buffer, item);
    }

    assert(initial_count == free_count);
    assert(0 == buffer->free(buffer));

    fprintf(stdout, "free() OK\n");

}

/*----------------------------------------------------------------------------*/
int main(int argc, char** argv) {

    cache = ringbuffer_create(31, count_free, &free_count);
    free_item = cache_free;
    free_item_additional_arg = cache;

    test_ringbuffer_create();
    test_capacity();
    test_add();
    test_pop();
    test_cached_ringbuffer_create();
    cache->free(cache);

    free_count = 0;
    cache = ringbuffer_create(31, count_string_free, &free_count);
    test_cached_free();
    cache->free(cache);

}

/*----------------------------------------------------------------------------*/
