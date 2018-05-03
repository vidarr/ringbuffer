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
#include "../src/buffercache.c"
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>

/*----------------------------------------------------------------------------
                                  ACTUAL TESTS
  ----------------------------------------------------------------------------*/

void test_buffercache_caching() {

    Ringbuffer* cache = buffercache_create(9);

    const size_t capacity = cache->capacity(cache);
    void* pointers[capacity];

    /* First step - fill buffer */
    for(size_t i = 0; i < capacity; ++i) {
        pointers[i] = buffercache_get_buffer(cache, i);
    }

    for(size_t i = 0; i < capacity; ++i) {
        buffercache_release_buffer(cache, pointers[i]);
    }

    /* No more buffers should be allocated - thus there should be 50 different
     * pointers appearing and no more -
     * we keep track in this array */

    memset(pointers, 0, sizeof(pointers));

    /* Shift some elements out of cache into ringbuffer */
    for(size_t i = 0; i < 1000 * cache->capacity(cache); ++i) {
        Buffer* item = buffercache_get_buffer(cache, i % capacity);
        assert(item);

        bool pointer_ok = false;

        size_t index_found = 0;

        for(size_t index = 0; index < capacity; ++index) {

            if(item == pointers[index]) {
                index_found = index;
                pointer_ok = true;
                break;
            }

            if(0 == pointers[index]) {
                index_found = index;
                pointers[index] = item;
                pointer_ok = true;
                break;
            }

        }

        assert(pointer_ok);
        assert(i % capacity <= item->capacity_bytes);

        if(i < capacity) {
            assert(i == index_found);
        }
        if(i > capacity) {
            assert(i > index_found);
        }


        buffercache_release_buffer(cache, item);
    }

    assert(0 == cache->free(cache));

    fprintf(stdout, "Caching ok\n");

}

/*----------------------------------------------------------------------------*/
int main(int argc, char** argv) {

    /* Caching tests */
    test_buffercache_caching();

}

/*----------------------------------------------------------------------------*/
