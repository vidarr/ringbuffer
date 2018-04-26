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


#include "../src/ringbuffer.c"
#include <stdio.h>
#include <assert.h>

/*----------------------------------------------------------------------------*/

void test_ringbuffer_create() {

    Ringbuffer* buffer = 0;

    assert(0 == ringbuffer_create(0, 0, 0));

    buffer = ringbuffer_create(1, 0, 0);
    assert(buffer);
    assert(capacity_func == buffer->capacity);
    assert(add_func == buffer->add);
    assert(pop_func == buffer->pop);
    assert(free_func == buffer->free);

    buffer = buffer->free(buffer);

    fprintf(stdout, "ringbuffer_create OK\n");

}

/*----------------------------------------------------------------------------*/

void test_capacity() {

    Ringbuffer* b = ringbuffer_create(1, 0, 0);
    assert(1 == b->capacity(b));
    b = b->free(b);

    b = ringbuffer_create(124, 0, 0);
    assert(124 == b->capacity(b));
    b = b->free(b);
}

/*----------------------------------------------------------------------------*/

void test_add() {

    int a = 1;
    int b = 2;
    int c = 3;

    Ringbuffer* buffer = 0;

    buffer = ringbuffer_create(1, 0, 0);
    assert(buffer->add(buffer, &a));
    assert(buffer->add(buffer, &b));
    assert(buffer->add(buffer, &c));
    buffer = buffer->free(buffer);

    buffer = ringbuffer_create(2, 0, 0);
    assert(buffer->add(buffer, &a));
    assert(buffer->add(buffer, &b));
    assert(buffer->add(buffer, &c));
    buffer = buffer->free(buffer);

    buffer = ringbuffer_create(20, 0, 0);
    assert(buffer->add(buffer, &a));
    assert(buffer->add(buffer, &b));
    assert(buffer->add(buffer, &c));
    buffer = buffer->free(buffer);

    fprintf(stdout, "add() OK\n");

}

/*----------------------------------------------------------------------------*/

void test_pop() {

    int a = 1;
    int b = 2;
    int c = 3;

    Ringbuffer* buffer = 0;

    buffer = ringbuffer_create(1, 0, 0);

    assert(buffer->add(buffer, &a));
    assert(&a == buffer->pop(buffer));
    assert(0 == buffer->pop(buffer));
    assert(0 == buffer->pop(buffer));
    assert(0 == buffer->pop(buffer));

    assert(buffer->add(buffer, &a));
    assert(buffer->add(buffer, &b));
    assert(&b == buffer->pop(buffer));
    assert(0 == buffer->pop(buffer));

    assert(buffer->add(buffer, &a));
    assert(buffer->add(buffer, &b));
    assert(buffer->add(buffer, &c));
    assert(&c == buffer->pop(buffer));
    assert(0 == buffer->pop(buffer));
    assert(0 == buffer->pop(buffer));
    assert(0 == buffer->pop(buffer));

    buffer = buffer->free(buffer);


    buffer = ringbuffer_create(2, 0, 0);

    assert(buffer->add(buffer, &a));
    assert(&a == buffer->pop(buffer));
    assert(0 == buffer->pop(buffer));
    assert(0 == buffer->pop(buffer));
    assert(0 == buffer->pop(buffer));

    assert(buffer->add(buffer, &a));
    assert(buffer->add(buffer, &b));
    assert(&a == buffer->pop(buffer));
    assert(&b == buffer->pop(buffer));
    assert(0 == buffer->pop(buffer));

    assert(buffer->add(buffer, &a));
    assert(buffer->add(buffer, &b));
    assert(buffer->add(buffer, &c));
    assert(&b == buffer->pop(buffer));
    assert(&c == buffer->pop(buffer));
    assert(0 == buffer->pop(buffer));
    assert(0 == buffer->pop(buffer));
    assert(0 == buffer->pop(buffer));
    assert(0 == buffer->pop(buffer));

    assert(buffer->add(buffer, &a));
    assert(buffer->add(buffer, &b));
    assert(&a == buffer->pop(buffer));
    assert(buffer->add(buffer, &c));
    assert(&b == buffer->pop(buffer));
    assert(buffer->add(buffer, &a));
    assert(&c == buffer->pop(buffer));
    assert(&a == buffer->pop(buffer));
    assert(0 == buffer->pop(buffer));

    buffer = buffer->free(buffer);

    fprintf(stdout, "pop() OK\n");

}

/*----------------------------------------------------------------------------*/

void count_free(void* int_pointer, void* count) {

    int* ip = (int*) int_pointer;
    free(ip);
    size_t* c = (size_t*) count;
    *c = *c + 1;

}
/*----------------------------------------------------------------------------*/

void test_free() {

    int a = 1;

    Ringbuffer* buffer = 0;
    assert(0 == free_func(0));

    buffer = ringbuffer_create(1, 0, 0);
    assert(0 == buffer->free(buffer));

    buffer = ringbuffer_create(21, 0, 0);
    assert(0 == buffer->free(buffer));

    buffer = ringbuffer_create(21, 0, 0);
    for(size_t i = 0; i < buffer->capacity(buffer); ++i) {
        buffer->add(buffer, &a);
    }
    assert(0 == buffer->free(buffer));

    size_t count = 0;
    buffer = ringbuffer_create(21, count_free, &count);
    for(size_t i = 0; i < buffer->capacity(buffer); ++i) {
        int* ip = calloc(1, sizeof(int));
        *ip = i;
        buffer->add(buffer, ip);
    }
    assert(0 == buffer->free(buffer));
    assert(21 == count);

    fprintf(stdout, "free() OK\n");

}

/*----------------------------------------------------------------------------*/
int main(int argc, char** argv) {

    test_ringbuffer_create();
    test_capacity();
    test_add();
    test_pop();
    test_free();

}

/*----------------------------------------------------------------------------*/
