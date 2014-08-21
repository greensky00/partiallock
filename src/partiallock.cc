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
#include <stdlib.h>
#include <string.h>

#include "partiallock.h"

struct plock_node {
    void *lock;
    void *start;
    void *len;
    volatile uint32_t wcount; /* waiting count */
    struct list_elem le;
};

void plock_init(struct plock *plock,
                struct plock_ops *ops,
                size_t sizeof_lock,
                size_t sizeof_range,
                void *aux)
{
    plock->ops = (struct plock_ops *)malloc(sizeof(struct plock_ops));
    *plock->ops = *ops;

    // allocate and init lock
    plock->sizeof_lock = sizeof_lock;
    plock->sizeof_range = sizeof_range;
    plock->aux = aux;
    plock->lock = (void *)malloc(plock->sizeof_lock);
    plock->ops->init(plock->lock);

    // init list and tree
    list_init(&plock->active);
    list_init(&plock->inactive);
}

plock_entry_t *plock_lock(struct plock *plock, void *start, void *len)
{
    struct list_elem *le = NULL;
    struct plock_node *node = NULL;

    // grab plock's lock
    plock->ops->lock(plock->lock);

    // find existing overlapped lock
    le = list_begin(&plock->active);
    while (le) {
        node = _get_entry(le, struct plock_node, le);
        if (plock->ops->is_overlapped(node->start, node->len, start, len, NULL)) {
            // overlapped
            // increase waiting count
            node->wcount++;
            // release plock's lock
            plock->ops->unlock(plock->lock);

            // grab node's lock
            plock->ops->lock(node->lock);
            // got control .. that means the owner released the lock
            // grab plock's lock
            plock->ops->lock(plock->lock);
            // decrease waiting count
            le = list_next(&node->le);
            node->wcount--;
            if (node->wcount == 0) {
                // no other thread refers this node
                // move from active to inactive
                list_remove(&plock->active, &node->le);
                list_push_front(&plock->inactive, &node->le);
            }
            // release node's lock
            plock->ops->unlock(node->lock);
        } else {
            le = list_next(le);
        }
    }

    // get a free lock
    le = list_pop_front(&plock->inactive);
    if (le == NULL) {
        // no free lock .. create one
        node = (struct plock_node *)malloc(sizeof(struct plock_node));
        node->lock = (void *)malloc(plock->sizeof_lock);
        plock->ops->init(node->lock);
        node->start = (void *)malloc(plock->sizeof_range);
        node->len = (void *)malloc(plock->sizeof_range);
    } else {
        node = _get_entry(le, struct plock_node ,le);
    }
    node->wcount = 0;

    // copy start & len value
    memcpy(node->start, start, plock->sizeof_range);
    memcpy(node->len, len, plock->sizeof_range);
    // insert into active list
    list_push_back(&plock->active, &node->le);

    // grab node's lock & release plock's lock
    plock->ops->lock(node->lock);
    plock->ops->unlock(plock->lock);

    return node;
}

void plock_unlock(struct plock *plock, plock_entry_t *plock_entry)
{
    struct list_elem *le = NULL;
    struct plock_node *node = plock_entry;

    // grab plock's lock
    plock->ops->lock(plock->lock);

    if (node->wcount == 0) {
        // no other thread refers this node
        // move from active to inactive
        list_remove(&plock->active, &node->le);
        list_push_front(&plock->inactive, &node->le);
    }
    plock->ops->unlock(node->lock);

    // release plock's lock
    plock->ops->unlock(plock->lock);
}

void plock_destroy(struct plock *plock)
{
    struct list_elem *le;
    struct plock_node *node;

    plock->ops->destroy(plock->lock);

    // free all active locks
    le = list_begin(&plock->active);
    while(le) {
        node = _get_entry(le, struct plock_node, le);
        le = list_next(le);

        // unlock and destroy
        plock->ops->unlock(node->lock);
        plock->ops->destroy(node->lock);
        free(node->start);
        free(node->len);
        free(node->lock);
        free(node);
    }

    // free all inactive locks
    le = list_begin(&plock->inactive);
    while(le) {
        node = _get_entry(le, struct plock_node, le);
        le = list_next(le);

        // destroy
        plock->ops->destroy(node->lock);
        free(node->start);
        free(node->len);
        free(node->lock);
        free(node);
    }

    // free plock
    free(plock->lock);
    free(plock->ops);
}

