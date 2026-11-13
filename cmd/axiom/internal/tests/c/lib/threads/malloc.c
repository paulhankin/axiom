/* The memory functions (C17 7.22.3) from many threads at once (7.22.3p2:
   as if each call were one indivisible operation): 16 threads allocate,
   fill, grow and free blocks of many sizes, calloc's are zero, and half of
   what each allocates another thread frees (handed over through a
   mutex-guarded queue). Every block keeps its contents. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#define THREADS 16
#define ROUNDS 3000
#define QCAP 64

static mtx_t qm;
static unsigned char *queue[QCAP];
static size_t qsize[QCAP];
static int qn, bad;

static int work(void *arg) {
    unsigned long long seed = (unsigned long long)(long)arg * 2654435761u + 1;
    unsigned char *keep[8] = { 0 };
    size_t ksize[8] = { 0 };
    int i, j;
    for (i = 0; i < ROUNDS; i++) {
        size_t n;
        unsigned char *p;
        int k = i % 8;
        seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
        n = (size_t)(seed >> 33) % (i % 50 == 0 ? 70000 : 300);
        if (keep[k]) {
            for (j = 0; j < (int)ksize[k]; j++)
                if (keep[k][j] != (unsigned char)(ksize[k] + j))
                    bad++;
            if (i % 3 == 0) {
                /* grown or shrunk: the common part stays */
                size_t m = ksize[k] / 2 + n;
                unsigned char *q = realloc(keep[k], m);
                size_t common = m < ksize[k] ? m : ksize[k];
                if (!q) {
                    bad++;
                    continue;
                }
                for (j = 0; j < (int)common; j++)
                    if (q[j] != (unsigned char)(ksize[k] + j))
                        bad++;
                free(q);
            } else {
                mtx_lock(&qm);
                if (qn < QCAP) {
                    queue[qn] = keep[k];
                    qsize[qn++] = ksize[k];
                    keep[k] = NULL;
                }
                mtx_unlock(&qm);
                if (keep[k])
                    free(keep[k]);
            }
            keep[k] = NULL;
        }
        p = i % 5 == 0 ? calloc(n, 1) : malloc(n);
        if (!p) {
            bad++;
            continue;
        }
        if (i % 5 == 0)
            for (j = 0; j < (int)n; j++)
                bad += p[j] != 0;
        for (j = 0; j < (int)n; j++)
            p[j] = (unsigned char)(n + j);
        keep[k] = p;
        ksize[k] = n;
        /* someone else's block */
        mtx_lock(&qm);
        if (qn > 0) {
            unsigned char *q = queue[--qn];
            size_t m = qsize[qn];
            mtx_unlock(&qm);
            for (j = 0; j < (int)m; j++)
                bad += q[j] != (unsigned char)(m + j);
            free(q);
        } else {
            mtx_unlock(&qm);
        }
    }
    for (i = 0; i < 8; i++)
        free(keep[i]);
    return 0;
}

int main(void) {
    thrd_t t[THREADS];
    int i;
    mtx_init(&qm, mtx_plain);
    for (i = 0; i < THREADS; i++)
        thrd_create(&t[i], work, (void *)(long)(i + 1));
    for (i = 0; i < THREADS; i++)
        thrd_join(t[i], NULL);
    while (qn > 0)
        free(queue[--qn]);
    printf("%d\n", bad); /* expect: 0 */
    return 0;
}
