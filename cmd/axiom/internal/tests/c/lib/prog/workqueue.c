/* A work queue of <threads.h> and <stdatomic.h>: four worker threads take
   tasks from a queue guarded by a mutex and two condition variables
   (bounded: producers wait when it is full), compute each task's result
   (an iterated hash), add it to an atomic total and an atomic count, and
   store it in its slot; the main thread produces the tasks, then a poison
   task per worker, joins them and checks every slot against a sequential
   computation. call_once initializes a table once; thread-specific
   storage counts each worker's tasks. */
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

enum { TASKS = 2000, WORKERS = 4, CAP = 16 };

static mtx_t lock;
static cnd_t not_empty, not_full;
static int queue[CAP], head, count;
static atomic_ullong total;
static atomic_int done;
static unsigned long long result[TASKS];
static once_flag once = ONCE_FLAG_INIT;
static unsigned table[256];
static atomic_int inits;
static tss_t mine;
static atomic_int per_worker_sum;

static void init_table(void) {
    unsigned i;
    for (i = 0; i < 256; i++)
        table[i] = i * 2654435761u;
    atomic_fetch_add(&inits, 1);
}

static unsigned long long work(int task) {
    unsigned long long h = (unsigned long long)task * 0x9E3779B97F4A7C15ULL;
    int i;
    for (i = 0; i < 2000; i++)
        h = (h ^ table[h & 255]) * 0x100000001B3ULL + (unsigned long long)i;
    return h;
}

static void put(int task) {
    mtx_lock(&lock);
    while (count == CAP)
        cnd_wait(&not_full, &lock);
    queue[(head + count) % CAP] = task;
    count++;
    cnd_signal(&not_empty);
    mtx_unlock(&lock);
}

static int take(void) {
    int task;
    mtx_lock(&lock);
    while (count == 0)
        cnd_wait(&not_empty, &lock);
    task = queue[head];
    head = (head + 1) % CAP;
    count--;
    cnd_signal(&not_full);
    mtx_unlock(&lock);
    return task;
}

static void count_done(void *p) {
    atomic_fetch_add(&per_worker_sum, *(int *)p);
    free(p);
}

static int worker(void *arg) {
    int *n = malloc(sizeof *n);
    (void)arg;
    *n = 0;
    tss_set(mine, n);
    call_once(&once, init_table);
    for (;;) {
        int task = take();
        unsigned long long r;
        if (task < 0)
            break;
        r = work(task);
        result[task] = r;
        atomic_fetch_add(&total, r);
        atomic_fetch_add(&done, 1);
        (*(int *)tss_get(mine))++;
    }
    return 0;
}

int main(void) {
    thrd_t t[WORKERS];
    int i, bad = 0;
    unsigned long long sum = 0;
    mtx_init(&lock, mtx_plain);
    cnd_init(&not_empty);
    cnd_init(&not_full);
    tss_create(&mine, count_done);
    for (i = 0; i < WORKERS; i++)
        if (thrd_create(&t[i], worker, NULL) != thrd_success)
            return 1;
    for (i = 0; i < TASKS; i++)
        put(i);
    for (i = 0; i < WORKERS; i++)
        put(-1);
    for (i = 0; i < WORKERS; i++)
        thrd_join(t[i], NULL);
    call_once(&once, init_table);
    for (i = 0; i < TASKS; i++) {
        unsigned long long r = work(i);
        sum += r;
        bad += r != result[i];
    }
    printf("done %d, mismatches %d, totals agree %d, table initialized %d time(s), per-worker counts sum %d\n",
           atomic_load(&done), bad, sum == atomic_load(&total), atomic_load(&inits), atomic_load(&per_worker_sum));
    printf("checksum %llu\n", sum);
    tss_delete(mine);
    cnd_destroy(&not_empty);
    cnd_destroy(&not_full);
    mtx_destroy(&lock);
    return 0;
}

// expect: done 2000, mismatches 0, totals agree 1, table initialized 1 time(s), per-worker counts sum 2000
// expect: checksum 5050620707490046256
