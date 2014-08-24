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

#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include "partiallock.h"
#include "arch.h"

void spin_init_wrap(void *lock) {
    spin_init((spin_t*)lock);
}

void spin_destroy_wrap(void *lock) {
    spin_destroy((spin_t*)lock);
}

void spin_lock_wrap(void *lock) {
    spin_lock((spin_t*)lock);
}

void spin_unlock_wrap(void *lock) {
    spin_unlock((spin_t*)lock);
}

int _int_is_overlapped(void *pstart1, void *plen1,
                       void *pstart2, void *plen2, void *aux)
{
    uint64_t start1, len1, start2, len2;
    start1 = *(uint64_t*)pstart1;
    len1 = *(uint64_t*)plen1;
    start2 = *(uint64_t*)pstart2;
    len2 = *(uint64_t*)plen2;

    if ((start1 + len1 > start2 && start2 >= start1) ||
        (start2 + len2 > start1 && start1 >= start2)) {
        // overlapped
        return 1;
    } else {
        return 0;
    }
}

struct sw_args {
    int wid;
    int n;
    int *array;
    uint64_t start;
    uint64_t len;
    struct plock *plock;
};

void* static_worker(void *voidargs)
{
    int i, j;
    plock_entry_t *plock_entry;
    struct sw_args *args = (struct sw_args *)voidargs;

    for (i=0;i<args->n;++i) {
        plock_entry = plock_lock(args->plock, &args->start, &args->len);
        // set
        for (j=args->start; j<args->len; ++j){
            args->array[j] = args->wid;
        }
        usleep(100);
        // integrity check
        for (j=args->start; j<args->len; ++j){
            assert(args->array[j] == args->wid);
        }
        plock_unlock(args->plock, plock_entry);
    }

    return NULL;
}

void static_test()
{
    int nthreads = 8;
    int nitrs = 10000;
    int i;
    int *array = (int *)alloca(sizeof(int) * 5 * (nthreads+1) + 1000);
    thread_t *tid = (thread_t*)alloca(sizeof(thread_t) * nthreads);
    void *ret;
    struct sw_args *args = (struct sw_args*)alloca(sizeof(struct sw_args) * nthreads);
    struct plock plock;
    struct plock_ops ops;

    ops = (struct plock_ops){spin_init_wrap,
                             spin_lock_wrap,
                             spin_unlock_wrap,
                             spin_destroy_wrap,
                             _int_is_overlapped};

    plock_init(&plock, &ops, sizeof(spin_t), sizeof(uint64_t), NULL);

    for (i=0;i<nthreads;++i) {
        args[i].wid = i;
        args[i].plock = &plock;
        args[i].n = nitrs;
        args[i].array = array;
        args[i].start = i*5;
        args[i].len = 10;
        thread_create(&tid[i], static_worker, &args[i]);
    }
    for (i=0;i<nthreads;++i){
        thread_join(tid[i], &ret);
    }

    plock_destroy(&plock);
}

int main(){
    static_test();
    return 0;
}

