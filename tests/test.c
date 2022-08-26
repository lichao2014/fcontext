#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "fcontext.h"
#include "list.h"

struct stack_context_t {
    void *sp;
    size_t size;
};

struct coroutine_pool_t;

struct coroutine_t {
    struct list_node_t hook;

    struct stack_context_t sc;

    void(*fn)(void *ud);
    void *ud;

    fcontext_t fctx;

    struct coroutine_pool_t *pool;

    int exit;
};

struct coroutine_pool_t {
    list_t runnings;
    struct coroutine_t main;
    struct coroutine_t *active;
};

void coroutine_resume(struct coroutine_t *co);

struct stack_context_t
stack_context_alloc() {
    struct stack_context_t sc;
    
    sc.size = 1024 * 1024;
    char *ptr = malloc(sc.size);
    sc.sp = ptr + sc.size;

    return sc;
}

void
stack_context_free(struct stack_context_t *sc) {
    char *ptr = (char *)sc->sp - sc->size;
    free(ptr);
}

struct coroutine_pool_t*
coroutine_pool_create() {
    struct coroutine_pool_t* pool = malloc(sizeof(struct coroutine_pool_t));
    if (pool) {
        list_init(&pool->runnings);

        memset(&pool->main, 0, sizeof(struct coroutine_t));
        pool->main.pool = pool;
        pool->main.exit = 0;

        pool->active = &pool->main;
    }

    return pool;
}

void
coroutine_pool_close(struct coroutine_pool_t *pool) {
    free(pool);
}

void
coroutine_pool_schedule(struct coroutine_pool_t *pool) {
    struct coroutine_t *co = &pool->main;
    if (!list_empty(&pool->runnings)) {
        co = list_of(pool->runnings.next, struct coroutine_t, hook);
        list_erase(&co->hook);
    }

    coroutine_resume(co);
}

void
coroutine_pool_yield(struct coroutine_pool_t *pool) {
    if (pool->active != &pool->main) {
        list_push_back(&pool->runnings, &pool->active->hook);
    }

    coroutine_pool_schedule(pool);
}


static void
fcontext_entry(transfer_t t) {
    struct coroutine_t *co = t.data;
    jump_fcontext(t.fctx, NULL);

    co->fn(co->ud);
    co->exit = 1;

    coroutine_pool_schedule(co->pool);
}

struct coroutine_t *
coroutine_create(struct coroutine_pool_t *pool,
                size_t stack_size,
                void(*fn)(void *),
                void *ud) {
    struct stack_context_t sc = stack_context_alloc();
    void *storage = (void *)((uintptr_t)((char *)sc.sp - sizeof(struct coroutine_t)) & ~0xff);

    struct coroutine_t *co = (struct coroutine_t *)storage;
    co->sc = sc;
    co->fn = fn;
    co->ud = ud;
    co->pool = pool;
    co->exit = 0;

    char *stack_top = (char *)storage - 64;
    char *stack_bottom = (char *)sc.sp - sc.size;
 
    co->fctx = make_fcontext(stack_top, stack_top - stack_bottom, &fcontext_entry);
    co->fctx = jump_fcontext(co->fctx, co).fctx;

    list_push_back(&pool->runnings, &(co->hook));

    return co;
}

void
coroutine_close(struct coroutine_t *co) {

}

static transfer_t empty_transfer;

static transfer_t
fcontext_ontop(transfer_t t) {
    struct coroutine_t *prev = t.data;
    if (prev->exit) {
        stack_context_free(&prev->sc);
    } else {
        prev->fctx = t.fctx;
    }

    return empty_transfer;
}

void
coroutine_resume(struct coroutine_t *co) {
    struct coroutine_t *prev = co->pool->active;
    co->pool->active = co;

    ontop_fcontext(co->fctx, prev, &fcontext_ontop);
}

struct coroutine_pool_t *g_pool;

static void
test(void *ud) {
    long long id = (long long)ud;

    for (int i = 0; i < 10; ++i) {
        printf("yield= id:%lld\n", id);
        coroutine_pool_yield(g_pool);
    }

    printf("finish id:%lld\n", id);
}

int 
main() {
    g_pool = coroutine_pool_create();

    coroutine_create(g_pool, 64 * 1024, test, (void *)1);
    coroutine_create(g_pool, 64 * 1024, test, (void *)2);

    while (1) {
        coroutine_pool_schedule(g_pool);
    }
}