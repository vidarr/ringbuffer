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

```C

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
Contrary to what you might have read in some beginners' bookss, putting *all*
declarations into the header is not a good practice at all, because *the
header is the interface* of the module, the header is what the users will see
of your module.
So in order to keep your interface simple and small, *put as few things in the
header as possible*.

This is the first beginners' rule of thumb we are going to break.

So, as we got our minimal interface already, lets put this together in a
header:

```C
#ifnder __RINGBUFFER_H__
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
much will change, we will even provide 2 implementations in parallel,
it will only grow with 2 additional functions.

The trick is, to achieve this...

## A first implementation

Ok, so we got our interface, now we need to implement the gory internals.
Whatever might come, we need to implement our functions defined in the header -
which is currently the constructor method only.
Thus, start of copying its prototype over to an empty source file and include
our glorious header:

```C
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

```C
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

```C
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

```C
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

```C
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
Gracefully, the C standard provides a neat feature:

*A pointer to a struct is the same as a pointer to it's first element* [4].

This means, that

```C
Ringbuffer* buffer = ringbuffer_create(1, 0, 0);

assert(buffer == buffer->capacity);
```

For example.

We can exploit that by *extending a structure without having to alter it's definition*:

```C
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

```C
InternalRingbuffer internal = calloc(1, sizeof(InternalRingbuffer));
Ringbuffer* ringbuffer = (Ringbuffer*) internal;
ringbuffer->add(ringbuffer, item);    /* Works perfectly */

```

This seems to be some kind of ugly kludge, but is totally portable and
standard conforming techinque.
The C standard guarantees this to work!
Since this is private stuff, this definition goes into the source file.

So, let's enhance our constructor function to create a 'working' ringbuffer:

```C
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
Again, gotos are in general a bad idea, except for handling error conditions.
If you think about it, try/catch blocks in object oriented languages are just
gotos in disguise.

Now, nothing prevents us from implementing `add()` and `pop()`:

```C
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

```C
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

```C
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

```C
static int EMPTY = 0;
...
void* pop_func(Ringbuffer* self) {

    ...
    void* retval = read->item;
    read->item = 0;
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

But there's more to come ...

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
