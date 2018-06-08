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
#include "../src/caching_ringbuffer.c"
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

/*----------------------------------------------------------------------------*/

static void count_free(void* item, void* additional_arg) {

    if(0 == additional_arg) goto error;

    size_t* counter = additional_arg;
    ++(*counter);

error:

    return;

}

/*----------------------------------------------------------------------------
                                  ACTUAL TESTS
  ----------------------------------------------------------------------------*/

void test_caching_ringbuffer_create() {

    Ringbuffer* buffer = 0;

    buffer = caching_ringbuffer_create(1, 0, 0);
    assert(buffer);
    assert(capacity_func == buffer->capacity);
    assert(add_func == buffer->add);
    assert(pop_func == buffer->pop);
    assert(free_func == buffer->free);

    buffer = buffer->free(buffer);

    fprintf(stdout, "caching ringbuffer_create OK\n");

}

/*----------------------------------------------------------------------------*/

void test_caching_ringbuffer_get_cached() {

    int a[10];

    assert(0 == caching_ringbuffer_get_cached(0));

    Ringbuffer* buffer = caching_ringbuffer_create(1, 0, 0);
    assert(0 == caching_ringbuffer_get_cached(buffer));
    assert(buffer->add(buffer, a));
    assert(buffer->add(buffer, a + 1));
    assert(buffer->add(buffer, a + 2));
    assert(a + 1 == caching_ringbuffer_get_cached(buffer));
    buffer = buffer->free(buffer);

    buffer = caching_ringbuffer_create(1, 0, 0);
    assert(0 == caching_ringbuffer_get_cached(buffer));
    assert(buffer->add(buffer, a));
    assert(buffer->add(buffer, a + 1));
    assert(buffer->add(buffer, a + 2));
    assert(a + 1 == caching_ringbuffer_get_cached(buffer));

    buffer = buffer->free(buffer);

    fprintf(stdout, "caching ringbuffer_get_cached OK\n");

}

/*----------------------------------------------------------------------------*/

void test_caching_free() {

    const size_t NUM_ELEMENTS = 114;
    size_t frees_count = 0;

    int test_data[NUM_ELEMENTS];

    for(size_t i = 0; i < NUM_ELEMENTS; ++i) {

        test_data[i] = i;

    }

    Ringbuffer* buffer =
        caching_ringbuffer_create(NUM_ELEMENTS / 2, count_free, &frees_count);

    size_t i = 0;

    while(10 > frees_count) {
        /* just some abritrary data that has been allocated dynamically */
        ++i;
        /* We want to create really new data buffers, thus cache == 0 ... */
        buffer->add(buffer, test_data + i);
    };

    const size_t initial_count = frees_count;

    /* Shift some elements out of cache into ringbuffer */
    for(i = 0; i < 3 * buffer->capacity(buffer); ++i) {
        void* item = caching_ringbuffer_get_cached(buffer);
        assert(item);
        buffer->add(buffer, item);
    }

    assert(initial_count == frees_count);
    assert(0 == buffer->free(buffer));

    fprintf(stdout, "free() OK: %zu\n", frees_count);

}

/*----------------------------------------------------------------------------*/
int main(int argc, char** argv) {

    /* The interface tests */
    size_t free_count = 0;

    create = caching_ringbuffer_create;

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
    test_caching_ringbuffer_create();
    test_caching_ringbuffer_get_cached();
    test_caching_free();

}

/*----------------------------------------------------------------------------*/
