/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
A Generic Partial Lock

Copyright (C) 2014  Jung-Sang Ahn <jungsang.ahn@gmail.com>
All rights reserved.

Last modification: Aug 20, 2014

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _JSAHN_PARITIAL_LOCK_H
#define _JSAHN_PARITIAL_LOCK_H

#include <stdint.h>
#include <stddef.h>

#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t plock_range_t;
typedef struct plock_node plock_entry_t; /* opaque reference */

struct plock_ops {
    void (*init_user)(void *lock);
    void (*lock_user)(void *lock);
    void (*unlock_user)(void *lock);
    void (*destroy_user)(void *lock);
    void (*init_internal)(void *lock);
    void (*lock_internal)(void *lock);
    void (*unlock_internal)(void *lock);
    void (*destroy_internal)(void *lock);
    int (*is_overlapped)(void *start1, void *len1, void *start2, void *len2, void *aux);
};

struct plock_config {
    struct plock_ops *ops;
    size_t sizeof_lock_user;
    size_t sizeof_lock_internal;
    size_t sizeof_range;
    void *aux;
};

struct plock {
    struct list active; /* list of active locks */
    struct list inactive; /* list of inactive (freed) locks */
    struct plock_ops *ops;
    size_t sizeof_lock_user;
    size_t sizeof_lock_internal;
    size_t sizeof_range;
    void *lock;
    void *aux;
};

void plock_init(struct plock *plock, struct plock_config *config);
plock_entry_t *plock_lock(struct plock *plock, void *start, void *len);
void plock_unlock(struct plock *plock, plock_entry_t *plock_entry);
void plock_destroy(struct plock *plock);

#ifdef __cplusplus
}
#endif

#endif

