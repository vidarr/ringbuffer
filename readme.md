# Advanced C Interfaces

## Abstract

In this article, we will provide advanced techinques to approach and master
complexity when developing massive C applications.

## Introduction

Modules are the key approach to mastering complexity [1].
Therefore, they should target maximum indenpendence.
A module should be as independent as possible from other code.
This will not only severly reduce complexity, but also allow you to create
effective unit tests.

The key in effectively moduralizing is designing good interfaces.

Good interfaces should aim at

- Simplicity: Restrict yourself to the minimum interface possible
  This not only fosters independence, but also enables users to use your module
  with ease. A simple interface provides fewer possibilities of errors,
  eases testing your interface thoroughly, and naturally reduces interdependence
  of your module with other code.

- Information hiding: Hide as much as possible of the internals of your module.
  This prevents users from abuse and clarifies the usage of your module.
  It also supports the interface to remain *stable*.

- Stability: Your internal implementation might change as rapidly as you desire,
  but the interface should be stable in order to prevent users from continuously
  having to port their code to new versions of your interface.


The remainder of this article will show some techniques on how to design for
proper modularity in C by developing a ringbuffer FIFO and improving upon it
step by step.

You will encounter certain practices that violate what you will have read in
some introductory book / article about C.
Just bear in mind that an introductory book is written for people totally
unfamiliar with C.
It totally makes sense to give some rule of thumbs for the beginner to give
orientation.
However, as in most scopes, rules of thumbs hold most of the time, but
at times it makes sense to break them.

## Basic design

As a small demonstration project, we want to implement a FIFO [2] as a
ringbuffer.

A ringbuffer is basically a FIFO, thus you can add items to it, and pop items
back out of it.
Key feature is that it will start overwriting the oldest items in the buffer.

We will use it as a FIFO, i.e. when popping, the oldest element will be
returned from the buffer (but you could use this structure as well as a
LIFO [3], popping out the newest element - however, I dont see an application
for a stack overwriting its oldest elements now).

What is the minimum a user requires to use this data structure?

- a function to add an item to the buffer: bool `add(Ringbuffer* r, void* item)`
- a function to pop out the oldest item: `void* pop(Ringbuffer* r)`
- Some kind of *constructor* method:
  `Ringbuffer* ringbuffer_create(
                            size_t capacity,
                            void (*free_item)(void* additional_arg, void* item),
                            void* additional_arg)`

    Dont wonder for now about the arguments to this function too much, it
    will become clear later on...
- Some function to free the ring buffer: `void* free(Ringbuffer* r)`

Now, wouldn't it be nice if we could provide different implementations of
a ringbuffer in a way to prevent users from adapting their code?
Maybe even allow for some kind of factory, that returnes different kinds
of ringbuffers, just as you know from factories in C++ or Java?
In other words, allow for some kind of inheritance?

We will therefore tie the functions as function pointers to our Ringbuffer.
Actually, the `Ringbuffer` itself needs to only be a collection of function
pointers (from the users' point of view, and that whats matters for now):

```c

typedef struct Ringbuffer {
    size_t        (*capacity) (struct Ringbuffer* self);
    bool          (*add)      (struct Ringbuffer* self, void* item);
    void*         (*pop)      (struct Ringbuffer* self);
    struct Ringbuffer* (*free)  (struct Ringbuffer* self);

} Ringbuffer;

```

(You might wonder why the label of the struct bears the same name as the type,
but that's ok - C got different namespaces for typedefs and struct labels -
and since both refer to the same item, don't bother coming up with a different
name)

Along with our constructor function to create such a `Ringbuffer`, this is
all a user requires to use our ringbuffer.

This is our interface.

In C, a module consists of a source file and a header.
Contrary to what you might have read in some beginners' books, putting *all*
declarations into the header is not a good practice at all, because *the
header is the interface* of the module, the header is what the users will see
of your module.
So in order to keep your interface simple and small, *put as few things in the
header as possible*.

This is the first beginners' rule of thumb we are going to break.

So, as we got our minimal interface already, lets put this together in a
header:

```c

#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

typedef struct Ringbuffer {
    size_t        (*capacity) (struct Ringbuffer* self);
    bool          (*add)      (struct Ringbuffer* self, void* item);
    void*         (*pop)      (struct Ringbuffer* self);
    struct Ringbuffer* (*free)  (struct Ringbuffer* self);

} Ringbuffer;

Ringbuffer* ringbuffer_create(
        size_t capacity,
        void (*free_item)(void* item, void* additional_arg),
        void* free_item_additional_arg);

#endif
```

That's it - that's our header. And surprisingly, in the course of this article,
it won't change much, we will even provide 2 implementations in parallel,
it will only grow with 2 additional functions.

The trick is, to achieve this...

## A first implementation

Ok, so we got our interface, now we need to implement the gory internals.
Whatever might come, we need to implement our functions defined in the header -
which is currently the constructor method only.
Thus, start of copying its prototype over to an empty source file and include
our glorious header:

```c

#include "../include/ringbuffer.h"

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

error:

    return 0;
}
```

And because we want to implement rock-solid code, write a neat unit test for
checking that the function does what it is supposed to:

```c

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

int main(int argc, char** argv) {

    test_ringbuffer_create();

}
```

And- you got your first program that compiles and will execute.
But of course, the unit test will fail since our constructor does not construct
anything. Lets make the test pass by extending your source code to:

```c

static size_t capacity_func(Ringbuffer* self);
static bool add_func(Ringbuffer* self, void* item);
static void* pop_func(Ringbuffer* self);
static Ringbuffer* free_func(Ringbuffer* self);

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

static size_t capacity_func(Ringbuffer* self) {
    return 0;
}

static bool add_func(Ringbuffer* self, void* item) {
    return false;
}

static void* pop_func(Ringbuffer* self) {
    return 0;
}

static Ringbuffer* free_func(Ringbuffer* self) {
    return self;
}
```

Now, the test passes as you actually create a Ringbuffer struct.
But, it's entirely usesless as it only provides dummy functions.

So, carry on by first thinking about the `add()` and `pop()` methods, then
implement unit tests to check for their proper functionality:

```c

...

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
    assert(&a == buffer->pop(buffer));
    assert(&b == buffer->pop(buffer));
    assert(&c == buffer->pop(buffer));
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

int main(int argc, char** argv) {

    test_ringbuffer_create();
    test_add();
    test_pop();

}

```

Again, the very first test will fail. That's actually good, because it means
that our tests are probably dumb, but actually can fail and therefore check
at least a little bit ;)

However, now we need to implement the `add()` and `pop()` functions.

The easiest implementation of a ringbuffer is a linked list, where the last
entry links back to the beginning (creating a ring - hence "ring buffer' ).

Thus we require some helper data type to store 1 item and a pointer to the
following entry:

```c

typedef struct Entry {
    void* datum;
    struct Entry* next;
}

```

Does the user require this struct? No, its an implementation detail that better
be hidden from the user - we could as well use an indexed array or whatever...
Thus lets store it in our source file.

Now, we need to store this linked list somewhere.
We do not want to make one global linked list for obvious reasons,
Naturally, we should attach it to our `Ringbuffer`.
So let's enhance it's definition - but: Does the user require direct access to
the internal data representation of a ringbuffer? Of course, this would be
violating all of our principles.
Thus, better not alter the definition of `Ringbuffer`.
But - how to attach data to a struct without altering it's definition?
Gracefully, the C standard provides a neat assertion:

*A pointer to a struct is the same as a pointer to it's first element* [4].

This means, that

```c

Ringbuffer* buffer = ringbuffer_create(1, 0, 0);

assert(buffer == (Ringbuffer*) & buffer->capacity);
```

For example.

We can exploit that by *extending a structure without having to alter it's definition*:

```c

typedef struct InternalRingbuffer {

    Ringbuffer public;

    Entry* next_entry_to_read;
    Entry* next_entry_to_write;
    size_t max_num_items;

    void (*free_item)(void* item, void* additional_arg);
    void* free_item_additional_arg;

} InternalRingbuffer;
```

We can safely cast a pointer to an `InternalRingbufer` to `Ringbuffer`
since a pointer to `InternalRingbuffer` equals a pointer to
 `InternalRingbuffer->public` which incidentally is a `Ringbuffer`:

```c

InternalRingbuffer internal = calloc(1, sizeof(InternalRingbuffer));
/* initialize internal */
...
Ringbuffer* ringbuffer = (Ringbuffer*) internal;
ringbuffer->add(ringbuffer, item);    /* Works perfectly */

```

This seems to be some kind of ugly kludge, but is totally portable and
standard conforming techinque.
The C standard guarantees this to work!
Since this is private stuff, this definition goes into the source file.

So, let's enhance our constructor function to create a 'working' ringbuffer:

```c

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
        .next_entry_to_write = list_start->next,
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
```

As you see, we use the dreaded 'goto' here.
Again, gotos are in general a bad idea, except under certain conditions.
If you think about it, try/catch blocks in object oriented languages are just
gotos in disguise.

Now, nothing prevents us from implementing `add()` and `pop()`:

```c

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
```

Now it becomes obvious why the 'ringbuffer_create()' function receives a
`item_free` function as argument - to free overwritten items
Providing the possibility to pass additional data to a callback is good
practice - you will see an example for the usefulness of this practice
in an instance.
What remains is the `free()` implementation to free a Ringbuffer again.
Again, implement a test first, then implement to pass the test.
This test wont be a unit test, because the main functionality is just freeing
obsolete allocations.
However, you can check for segfaults and have the test run with valgrind(1) or
other memory leak detection tools:

```c

void count_free(void* int_pointer, void* count) {

    int* ip = (int*) int_pointer;
    free(ip);
    size_t* c = (size_t*) count;
    *c = *c + 1;

}

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
```

Here you see a first example of the handyness of providing the possibility to
pass over arbitrary additional data to handlers like the `free_item()` function.

The last thing to do is actually implement the `free()`:

```c

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
```

Now the `capacity()` function and tests are still missing, but that's a
real nobrainer and you might just look it up in the git repo source if
you really feel you need to have a look...

A slight remark: We use null pointers in here to mark an entry empty.
This is ok, but prevents users from storing null as an item in the buffer -
whyever one would like to do so.
You could work around this by just declaring a static variable and using its
mem address to mark an entry empty:

```c

static int EMPTY = 0;
...
void* pop_func(Ringbuffer* self) {

    ...
    void* retval = read->item;
    read->item = &EMPTY;
    ...
    /* etc */
```

However, in here I dont see the need to do so.

Now we achieved total hiding of our internals from the user, enabling us to
re-arrange, re-implement whatever we desire under the hood of our Ringbuffer
without the user being required to adapt to it at all, thanks to our clean,
minimal and implementation independent test cases.
The user can rely on the interface not to change, because everything we might
have to change in order to gain speed e.g. is inaccessible by the user.

He will not be tempted to understand the gory details, enabling it to rapidly
use our Ringbuffer, because of the simplicity of its interface.

One slight drawback in here is the need to stay flexible we have to resort back
to `void*` types. This is in general nasty: It disables one of the key
advantages that statically typed languages provide over dynamically typed ones
like Python: The ability to check *at compiletime*, thus *without runtime
overhead*, avoid an entire class of programming errors.

Unfortunately, in C `void*` cannot be avoided all the time.
As always, there's always tradeoffs: Here we trade type safety for flexibility.
Do this with care!

## A remark on unit tests

You will have noticed that the tests we implemented only test against the
*interface*, not the internals of the `Ringbuffer`.
It's totally alright to also test internals, but beware of mixing interface
tests with testing internals.
Our tests work with *any implementation compliant with the interface
definition* .
If we decide to move from a linked list to a C array, we can still use the
interface tests to verify the other implementation.
Thus, no matter what you do, *keep interface tests separated from internal
tests*.

/*0*/

## Use the ringbuffer as a cache

So we got a neat ringbuffer, that will receive data and overwrite old
data on insuffient memory.
Lets try to use our ring buffer in some way.
What could this actually be useful for?
We already discussed that it is perfectly suited for a stream of data has to
be handed over to another thread for media streaming e.g.

However, such an application is complicated and rather huge, thus lets focus
on something different:

Assume you got a time-critical application, and you did already some profiling
and optimisation.

As it turns out, at some point you will notice that calling `malloc(3)` or
`calloc(3)` does not come for free but consumes some processor time.
Why is this the case?
In fact, the operating system does not really provide for a memory manager
that you can query for, lets say, 90 bytes.
Your process receives a dedicated address space, which in theory ranges from

`0x0` to `0xffff ffff ffff ffff` on a 32 bit system.
However, most of the addresses will not be existing for real,
 it's your process *virtual address space*.
Your process will be given a certain, contiguous block of *real* RAM memory,
 mapped into its virtual address space, let's say in betwen `0xffff` and
 `0xff00 0000`.
All you can ask the system for is raising the bounds of this
*real memory window*, let's say from `0xff00 0000` to `0xff00 ffff`.
That is, however, not how `malloc(3)` behaves: you *can* ask `malloc(3)`
for arbitrary chunks of memory.
Under the hood, `malloc(3)` performs quite a bit of management:
It keeps track of free memory blocks.
If you request a certain number of bytes, `malloc(3)` will sweep through it's
list looking for a suitable block.
Often enough, it won't find one that fits the requested size *exactly*, thus it
will have to do some adjustments, either shrink a block, or claim a raise
of the memory bounds as discussed above.
If you free a block, it will put this block back into the list of available
blocks.
In order to counter total fragmentation, it will try to merge to adjacent
available blocks into one bigger one.
One could call it a poor man's crippled garbage collector.

A lot of stuff that usually goes unnoticed - And costs time.
Thus at some point of optimisation, it pays off to avoid *mallocs*.
How can this be achieved?
One trait to go is recycling of memory blocks.

Let's look at an example:

Assume you store your app data in buffers like

```c

typedef struct {

    size_t capacity_bytes;
    size_t bytes_used;
    uint8_t* data;

} Buffer;

```

You read in data from some I/O device like so

```c

Buffer* db = buffer_create(255);
db->bytes_used = read(fd, db->data, db->capacity_bytes);

process_data(db);

buffer_free(db);
db = 0;
```

You could avoid allocating and freeing your data buffers if you do this several
times by just allocating *one* data buffer and keep reusing it like:

```c

Buffer* db = buffer_create(255);

...

while(go_on_reading) {

    if(num_bytes_to_read > db->capacity_bytes) {
        free(db->data);
        db->data = malloc(num_bytes_to_read);
        db->capacity_bytes = num_bytes_to_read;
    }

    db->bytes_used = 0;

    db->bytes_used = read(fd, db->data, db->capacity_bytes);

    process_data(db);

}

buffer_free(db);
db = 0;

```

That's fine, but what if your program does not consist of a single big loop, but
is more complicated, allocates and frees DataBuffers on various locations in
the code?

Here, our Ringbuffer pops in - it's perfectly suited for caching.
the basic idea is that `data_buffer_free` actually adds the DataBuffer to free
into a Ringbuffer, and `data_buffer_create` first of all tries to pop a
DataBuffer from the Ringbuffer.

you could then write the upper loop e.g. like this:

```c

Ringbuffer* cache = 0;
cache = ringbuffer_create(NUM_DATABUFFERS_TO_CACHE, data_buffer_free, 0);

Buffer* db = 0;

...

while(go_on_reading) {


    db = buffercache_get_buffer(cache, 255);
    db->bytes_used = read(fd, db->data, db->capacity_bytes);

    process_data(db);

    buffercache_release_buffer(cache, db);
    db = 0;

}

```

That looks quite simpler as the second loop example.
Moreover, you can easily cache all your DataBuffers whereever you use them
just by calling either `buffercache_get_buffer()` or `buffercache_release_buffer()`.

So, let's implement a cache for DataBuffers.
Sticking to our dogma, first design the interface.
In this case, it's more or less obvious wen looking at the example above:

```C

Ringbuffer* buffercache_create(size_t capacity);
Buffer* buffercache_get_buffer(Ringbuffer* cache, size_t min_size_bytes);
bool buffercache_release_buffer(Ringbuffer* cache, Buffer* buffer);

```

And design a proper test:

```C

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

    memset(pointers, 0, sizeof(pointers) * sizeof(void*));

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


```

This is, of course, not a perfect test, since it does not check all
aspects of the interface.

The implementation of those three methods will be rather straight forward:

```c

static void buffer_free(void* data_buffer, void* arg);

/******************************************************************************
                               PUBLIC FUNCTIONS
 ******************************************************************************/

Ringbuffer* buffercache_create(size_t capacity) {

    return ringbuffer_create(capacity, buffer_free, 0);

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

    buffer->bytes_used = 0;

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

```

That's all that is to facilitating a Ringbuffer as a cache.

Notice that in this interface, we did away with the `void*` nuisance by
hiding it.
This does not solve the main problem - the implementation of our buffer cache
still struggles with types unsafety, but it does not forward this unsafe
bit to the user of our interface.
We cannot get rid of this problem, but we can prevent the users of our interface
to have to deal with it, at least.

### Writing simple code: Nested Ifs

Notice the particular control flow in these functions, particularly in
`buffercache_get_buffer()`:

In my opinion, it does not pay off to nest ever deeper levels of
`if`.
In fact, I suggest to test one condition after the other, without nesting.

This simplifies understanding the code, since you can turn your code into
a rather simple list, where you ensure one condition, one after the other,
building slowly towards your final goal.

Have a look at the `data_buffer_get()` function:

1. It tries to pop a data buffer from the cache if there is a cache.
2. If we do not have a data buffer yet, it tries to allocate a new one - no matter if the popping failed or for whatever reason there is not data buffer yet.
3. Now we know that we have a data buffer, but we still do not know its state - does it have some `data` allocated already? Is the `data` array large enough?
4. Again, treat one problem after the other: If the data array is too small, free it and set `data` to 0
5. If `data` is 0, allocate it with a sufficient size. Again, it does not matter whether it is 0 because we allocated the data buffer freshly or because we freed it due to its insufficient size.
6. Now we know that no matter how we came here, we got a data buffer, it is properly initialized and has sufficent capacity.

Also, avoid else branches by just short-cutting behind the sequence of ifs
by using a goto.

Gotos are considered harmful, and I consider this true as well - if used by
beginners or in abundance or uncontrolled manner.
Gotos are like a toxic like digitalis: It should be avoided by the not knowing.
But experts can do great good with small amounts of it.

One use case is to have a sequence of `if` statements, and just use gotos
in their bodies to skip over the remainder of the ifs - avoiding complicated
else constructs.

Instead of doing something like

```c

if(a) {
    if(b &&  !c) {
        do_b_and_not_c();
    } else {
        if(d) {
            do_not_b_or_c_and_d();
        }
        do_not_b_or_c();
    }
} else {
    do_not_a();
}
```

do it like this:

```c

    if( ! a) {
        do_not_a();
        goto finish;
    }

    assert(a); /* we do not need the if(a) any more, becaus a must be true in here */

    if(b && ! c) {
        do_b_and_not_c();
        goto finish;
    }

    if(d) {
        do_not_b_or_c_and_d();
    }

    do_not_b_or_c();

finish:

    /* and we are done */
```

At least in my humble opinion, the second version is far easier to understand,
despite the dreaded gotos.

Do not misunderstand: If in doubt, relinquish from using gotos.
I am not in doubt in this example, though.

I cannot give an algorithm on how to transform *any* structure of nested ifs
into a straight one-level if sequence nor can I prove that this is always
possible.
In my experience, however, I never stumbled upon a problem that forced me
to code some beast like the first version up there.

My suggestion is:
If you encounter yourself writing nested ifs, take some time to consider
if there is a way to streamline this part of your code into a single-level
if sequence. It will turn out to work surprisingly often.

# Switching the implementation

Now, if we want a ringbuffer to use a cache, i.e. if having to overwrite
elements, instead of freeing them having it put the element into a cache?

Easy enough: Just give `ringbuffer_create` the cache as `additional_arg`,
and `buffercache_release_buffer` as `free_item` function.
Ok, not quite that easy: Since `buffercache_release_buffer` and `free_item` got
different signatures, we require an 'adapter' method:


```c
void cache_free(void* item, void* cache) {

   /* strictly speaking superfuous since buffercache_release_buffer checks
      again, but better check too often than to seldomly */
    if(0 == cache) goto finish;
    if(0 == item) goto finish;

    if(! buffercache_release_buffer((Ringbuffer*) cache, (Buffer*) buffer)) {

        fprintf(stderr, "Freeing element failed\n");

    }

finish:

    do{}while(0);

}

Ringbuffer* cache = ...;

Ringbuffer* caching_ringbuffer = ringbuffer_create(20 cache_free, cache);
```

Voila, you got your cache, and whenever an item is going to be overwritten
in caching_ringbuffer, it will instead be transferred back into the cache.

But what if, for some reason, you do not want to expose the cache,
but suppose you want a ring buffer that brings its own internal cache, i.e.
*is* its own cache, and yielding an interface like:

```c

Ringbuffer* caching_ringbuffer_create(
        size_t capacity,
        void (*free_item)(void* item, void* additional_arg),
        void* free_item_additional_arg);

/*----------------------------------------------------------------------------*/

void* caching_ringbuffer_get_cached(Ringbuffer* crb);

/*----------------------------------------------------------------------------*/

bool caching_ringbuffer_release(Ringbuffer* crb, void* item);

```

Its debatable whether this really is a good idea, but that's not the point
we want to reach.

First of all, we need to use a different internal structure for our ringbuffer
since it must also contain the cache:

```c

typedef struct InternalRingbuffer {

    Ringbuffer public;
    Ringbuffer* cache;

} InternalRingbuffer;

```

But wait a second - the `public` Ringbuffer interface only contains the
interface - where will all the internals of an ordinary Ringbuffer go, like our
`next_entry_to_read` pointer?

The next approach

```c

typedef struct InternalRingbuffer {

    InternalRingbuffer super;
    Ringbuffer* cache;

} InternalRingbuffer;

```

is no solution either because we spent a lot of effort ensuring that
`InternalRingbuffer` is not visible outside our ringbuffer module.

But, what we can do is just the same as we do with the cache - *wrapping*
the actual ringbuffer like so:

```c

typedef struct InternalCachingRingbuffer {

    Ringbuffer public;
    Ringbuffer* buffer;
    Ringbuffer* cache;

} InternalCachingRingbuffer;

```

Let's implement it using our plain old procedure: Write some unit tests first:

```c

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

}

```

Note that we use our already existing `test_capacity`, `test_add` and `test_pop`
test functions since they just check the interface and this did not change.

Go on implementing the 'constructor':

```c

Ringbuffer* caching_ringbuffer_create(
        size_t capacity,
        void (*free_item)(void* item, void* additional_arg),
        void* free_item_additional_arg) {

    InternalRingbuffer* internal = calloc(1, sizeof(InternalRingbuffer));
    internal->cache =
    ringbuffer_create(capacity, free_item, free_item_additional_arg);
    internal->buffer = ringbuffer_create(capacity, cache_free, internal->cache);

    return (Ringbuffer*) internal;

}

```

But if we execute the tests, we encounter `SEGFAULTS`.
The problem is, if we look at this code:

```c

Ringbuffer* rb = caching_ringbuffer_create(10, my_free, 0);
rb->add(rb, my_element);

```

But nobody ever initialized `rb->add` !

What should be done here is obvious: `rb->buffer->add(rb->buffer, my_element)`.
We already have seen a solution for this: *Wrapping*:

```c

static size_t capacity_func(Ringbuffer* self);

static bool add_func(Ringbuffer* self, void* item);

static void* pop_func(Ringbuffer* self);

static Ringbuffer* free_func(Ringbuffer* self);

/*----------------------------------------------------------------------------*/

Ringbuffer* caching_ringbuffer_create(
        size_t capacity,
        void (*free_item)(void* item, void* additional_arg),
        void* free_item_additional_arg) {

    InternalRingbuffer* internal = calloc(1, sizeof(InternalRingbuffer));
    internal->cache =
    ringbuffer_create(capacity, free_item, free_item_additional_arg);
    internal->buffer = ringbuffer_create(capacity, cache_free, internal->cache);

    Ringbuffer->public = (Ringbuffer) {
        .capacity = capacity_func,
        .add = add_func,
        .pop = pop_func,
        .free = free_func,
    };

    return (Ringbuffer*) internal;

}

/*----------------------------------------------------------------------------*/

static size_t capacity_func(Ringbuffer* self) {

    if(0 == self) goto error;

    InternalRingbuffer* internal = (InternalRingbuffer*) self;
    Ringbuffer* buffer = internal->buffer;

    return buffer->capacity(buffer);

error:

    return 0;

}

/*----------------------------------------------------------------------------*/

static bool add_func(Ringbuffer* self, void* item) {

    if(0 == self) goto error;

    InternalRingbuffer* internal = (InternalRingbuffer*) self;
    Ringbuffer* buffer = internal->buffer;

    if(0 == buffer) goto error;

    return buffer->capacity(buffer);

error:

    return 0;

}

/*----------------------------------------------------------------------------*/

static void* pop_func(Ringbuffer* self) {

    if(0 == self) goto error;

    InternalRingbuffer* internal = (InternalRingbuffer*) self;
    Ringbuffer* buffer = internal->buffer;

    if(0 == buffer) goto error;

    return buffer->pop(buffer);

error:

    return 0;

}

/*----------------------------------------------------------------------------*/

static Ringbuffer* free_func(Ringbuffer* self) {

    if(0 == self) goto error;

    InternalRingbuffer* internal = (InternalRingbuffer*) self;
    Ringbuffer* buffer = internal->buffer;

    if(0 == buffer) goto error;

    internal->buffer = buffer->free_func(buffer);

    buffer = internal->cache;
    internal->cache = buffer->free_func(buffer);

    free(self);

    return 0;

error:

    return self;
}

```

# A friendly warning

Although these methods look really elegant, there are, as with everything,
drawbacks which you should always bear in mind:

Indirections over pointers cause overhead, and this causes potential
performance hits.

If you decide to call a function

    void f(13);

directly like

    f(13);

what happens is that the linker/exe loader knows the position of `f` in memory
and will insert its memory address directly at the position in the executable
where the function is called.

Fire up cgdb like

    $ cgdb ringbuffer_test

and let's look at the assembly code for our `free_func` method:

    (gdb) disassemble  free_func
    ...
    0x0000000000000aa8 <+108>:	mov    (%rdx),%rdx
    0x0000000000000aab <+111>:	mov    %rcx,%rsi
    0x0000000000000aae <+114>:	mov    %rdx,%rdi
    0x0000000000000ab1 <+117>:	callq  *%rax
    0x0000000000000ab3 <+119>:	mov    -0x8(%rbp),%rax
    0x0000000000000ab7 <+123>:	mov    %rax,%rdi
    0x0000000000000aba <+126>:	callq  0x620 <free@plt>
    0x0000000000000abf <+131>:	mov    -0x20(%rbp),%rax
    0x0000000000000ac3 <+135>:	mov    %rax,-0x8(%rbp)
    0x0000000000000ac7 <+139>:	mov    -0x8(%rbp),%rax
    0x0000000000000acb <+143>:	cmp    -0x18(%rbp),%rax
    0x0000000000000acf <+147>:	jne    0xa6f <free_func+51>
    0x0000000000000ad1 <+149>:	mov    -0x28(%rbp),%rax
    0x0000000000000ad5 <+153>:	mov    %rax,%rdi
    0x0000000000000ad8 <+156>:	callq  0x620 <free@plt>
    0x0000000000000add <+161>:	movq   $0x0,-0x28(%rbp)
    0x0000000000000ae5 <+169>:	jmp    0xae8 <free_func+172>
    0x0000000000000ae7 <+171>:	nop
    0x0000000000000ae8 <+172>:	mov    -0x28(%rbp),%rax
    0x0000000000000aec <+176>:	leaveq
    0x0000000000000aed <+177>:	retq

Notice `callq  0x620 <free@plt>` which is the call to `free()`: The memory address
of the `free` function has been inserted into the code directly.

And notice `callq *%rax` - this is the code generated for the call to
`internal->free_item(...)` .
There is no direct memory address, instead, the program had to store
the memory address in the register `rax` and have the processor load the memory
address of the function to call from this address: An additional load operation
from memory is required to call a function via a function pointer.

This might not seem disastrous, and indeed, it usually is negletible.
But for high performance applications on embedded systems, it might become
a nuisance.
Just bear it in mind and if in doubt, perform benchmarks to figure out the
actual performance hit if you suspect the hit to become relevant for your app.

This holds all the more if you perform wrapping extensively like in our
`caching_ringbuffer` object. the pop function is called indirectly, and then
calls another pop function indirectly.
instead of one function call, we end up with *two* function calls and *two*
loads from memory.

# So how should one go on?

After showing this approach of 'pulling down' OOP to C,
should one just begin to try to wrap *abything* like that?

The most important lesson I learned this far is: *There is no 'the one and true solution'*
that yields best results in every situation - nowhere, and especially not in
software development.

There are a few principles one *should* stick to, like modularity, separation
and encapsulation.

But there are usually different paths that let you achieve those.
Think of functional vs. procedural vs. object oriented programming.
Since the 60s, academics stuck with LISP, a highly functional language - they
considered it the true way.
They achieved quite a bit, especially in developing language properties that
modern languges often present us as revolutionary: Incremental garbage collection,
just-in-time compilation and reflection.
Their notion of the unit to manage code and data in was the *closure*.
Then object-orientation took over in the late 80ies, with C++ being developed.
LISP was considered poor in performance and just too hard to map onto real-life
stack machines.
Compilation was considered the way to go, and classes became the unit in which
to manage code and data in.

Nowadays, the benefits of closures and their little siblings, lambda expressions
have been re-discovered.
Honestly, I like to play around and do pure functional programming - that's why
I implemented my own [LISP interpreter](https://github.com/vidarr/fulisp),
but I don't see the big advantages why they even fucked up C++ with these
concepts (honestly, C++ has been fucked up since the very beginning - C is hard
to get right, undefined/implemention dependent behaviour all around the place -
and C++ levered all these pitfalls one stage, wrapped dozens of layers of
complexity around, tried to fix some things and fucking the language up even more - think
of references that are promised to always point to a valid object, unless you
let them point at an invalid one...).

# Advises & rules of thumb

That being said, wrapping your data *and* code into structs like that is
a highly useful approach *under certain circumstances*.
Meanwhile, it is an aweful thing to do *under certain circumstances*.

It will be beneficial to wrapping your code like this if

* You deal with methods that are likely to change, perhaps even at runtime.
  It could be like in our example, where you got an entity with defined behaviou,
  but the actual implementation is likely to change. 'Collections' like lists
  etc. naturally fall under this group - think of a linked list vs. a list
  implemented with an array. None is better, both have their advantages /
  disadvantages. Thus better keep the code using those entities independed
  of the implementation.

* Or you have a variety of objects that should provide a common interface
  to work with our app out-of-the-box due to the common interface.
  Device drivers would be a good example.

# Footnotes & References

[1] This approach is far more universal than just for software engineering -
for analyzing complex systems, the general approach is to separate the system
as simple and independent parts, and minimize the interactions between them.

[2] First-in-First-Out. typical applications are queues,
e.g. for communicating between threads. Actually, this very ringbuffer stems
a audio streaming software project where it is used to pass PCM data between
threads.

[3] Last-In-First-Out. Typically called 'stack' and used for parsing all kinds
of languages, in interpreters and compilers, but also within the processor
to store local variables and handling function calls and returns.

[4] Draft C11 Standard, ISO/IEC 9899:201x, 6.7.2.1, Semantics, paragraph 15: 
"A pointer to a structure  object,  suitably  converted,  points  to  its
initial  member  (or  if  that  member  is  a bit-field,  then  to  the  unit
in  which  it  resides),  and  vice  versa.   There  may  be  unnamed padding
within a structure object, but not at its beginning."
