// NOTE: please read comments!
#include "malloc.h"

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "trace.h"

#define PTR_ADD(p, offset) (((char *) p) + offset)
#define PTR_SUB(p, offset) (((char *) p) - offset)

typedef unsigned int word_t;

#define WSIZE sizeof(word_t)
#define DSIZE (WSIZE << 1)

#define GET(p) (*(word_t *)(p))
#define PUT(p, val) (*(word_t *)(p) = (word_t)(val))

#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

#define ALIGNED_SIZE(size, unit) (unit * ((size + (unit - 1)) / unit))
// This is the code from the seminar
typedef struct {
    int lock;
    size_t pagesize;
    void *start;
    void *end;
    void *head;
    size_t alloc;
    size_t payload;
    size_t usage;
    // we define void pointer to track last search location
    void *last;
} heap_t;

static heap_t g_heap = {0};

#define HEAP_LOCK while(__atomic_test_and_set(&g_heap.lock, __ATOMIC_ACQUIRE)) {}
#define HEAP_UNLOCK __atomic_clear(&g_heap.lock, __ATOMIC_RELEASE);

void heap_dump() {
    void *hdr = PTR_SUB(g_heap.head, WSIZE);
    word_t hdata = GET(hdr);
    size_t hsize;
    char buff[64] = {0};
    while ((hsize = (hdata & ~0x7))) {
        void *ptr = PTR_ADD(hdr, WSIZE);
        void *ftr = PTR_ADD(hdr, hsize - WSIZE);
        word_t fdata = GET(ftr);
        size_t fsize = fdata & ~0x7;
        int len = snprintf(
                buff,
                sizeof(buff),
                "%p = [%zu/%d : %zu/%d]\n", ptr, hsize, hdata & 1, fsize, fdata & 1
        );
        write(STDOUT_FILENO, buff, len);
        hdr = PTR_ADD(hdr, hsize);
        hdata = GET(hdr);
    }
}

void stat_dump() {
    size_t heapsize = g_heap.end - g_heap.start;
    size_t allocated = g_heap.alloc;
    size_t free = heapsize - allocated;
    long usage = (100 * allocated) / heapsize;
    long payload = (g_heap.payload * 100) / g_heap.usage;
    char buff[256];
    int len = snprintf(
            buff,
            sizeof(buff),
            "Heap    = %zu\nAlloc   = %zu\nFree    = %zu\nUsage   = %ld%%\nPayload = %ld%%\n",
            heapsize,
            allocated,
            free,
            usage,
            payload
    );
    write(STDOUT_FILENO, buff, len);
}

static void heap_init() {
    heap_t heap = {0};
    heap.pagesize = sysconf(_SC_PAGESIZE);
    heap.start = sbrk(heap.pagesize);
    heap.end = PTR_ADD(heap.start, heap.pagesize);
    heap.head = PTR_ADD(heap.start, DSIZE);
    // we also define heap.last 
    heap.last = heap.head;
    size_t bsize = heap.pagesize - DSIZE;
    PUT(heap.start, 1);
    PUT(PTR_ADD(heap.start, WSIZE), bsize);
    PUT(PTR_SUB(heap.end, DSIZE), bsize);
    PUT(PTR_SUB(heap.end, WSIZE), 1);
    g_heap = heap;
    trace("malloc is initialized\nstart=%p\nend=  %p\nhead= %p\n",
          heap.start, heap.end, heap.head);
}

static void *heap_extend(size_t size) {
    size_t asize = ALIGNED_SIZE(size, g_heap.pagesize);
    void *start = sbrk(asize);
    void *end = PTR_ADD(start, asize);
    PUT(PTR_SUB(start, WSIZE), asize);
    PUT(PTR_SUB(end, DSIZE), asize);
    PUT(PTR_SUB(end, WSIZE), 1);
    g_heap.end = end;
    return start;
}

static void place(void *ptr, size_t asize) {
    void *hdr = PTR_SUB(ptr, WSIZE);
    size_t bsize = GET_SIZE(hdr);
    void *ftr = PTR_ADD(hdr, bsize - WSIZE);
    size_t rsize = bsize - asize;
    if (rsize >= DSIZE) {
        void *rhdr = PTR_ADD(hdr, asize);
        void *aftr = PTR_SUB(rhdr, WSIZE);
        PUT(hdr, asize | 1);
        PUT(aftr, asize | 1);
        PUT(rhdr, rsize);
        PUT(ftr, rsize);
    } else {
        PUT(hdr, bsize | 1);
        PUT(ftr, bsize | 1);
    }
}

static void *coalesce(void *ptr) {
    void *hdr = PTR_SUB(ptr, WSIZE);
    size_t size = GET_SIZE(hdr);
    void *prev_ftr = PTR_SUB(hdr, WSIZE);
    void *next_hdr = PTR_ADD(hdr, size);
    word_t prev_alloc = GET_ALLOC(prev_ftr);
    word_t next_alloc = GET_ALLOC(next_hdr);
    if (prev_alloc && next_alloc) { // Case 1
        return ptr;
    }
    void *ftr = PTR_SUB(next_hdr, WSIZE);
    size_t prev_size = GET_SIZE(prev_ftr);
    size_t next_size = GET_SIZE(next_hdr);
    void *prev_hdr = PTR_SUB(hdr, prev_size);
    void *next_ftr = PTR_ADD(ftr, next_size);
    if (prev_alloc && !next_alloc) {
        size += next_size;
        PUT(hdr, size);
        PUT(next_ftr, size);
    } else if (!prev_alloc && next_alloc) {
        size += prev_size;
        PUT(prev_hdr, size);
        PUT(ftr, size);
        ptr = PTR_ADD(prev_hdr, WSIZE);
    } else {
        size += prev_size + next_size;
        PUT(prev_hdr, size);
        PUT(next_ftr, size);
        ptr = PTR_ADD(prev_hdr, WSIZE);
    }
    return ptr;
}

void *malloc(size_t size) {
    trace("malloc(%zu)\n", size);
    if (size == 0) {
        return NULL;
    }
    size_t asize = ALIGNED_SIZE(size, DSIZE) + DSIZE;
    HEAP_LOCK
    void *ptr = find_fit(asize);
    if (!ptr) {
        ptr = heap_extend(asize);
        ptr = coalesce(ptr);
    }
    place(ptr, asize);
    g_heap.alloc += asize;
    g_heap.usage += asize;
    g_heap.payload += size;
    HEAP_UNLOCK
    trace("malloc(%zu) = %p\n", size, ptr);
    stat_dump();
    return ptr;
}

void free(void *ptr) {
    trace("free(%p)\n", ptr);
    if (!ptr) {
        return;
    }
    void *hdr = PTR_SUB(ptr, WSIZE);
    HEAP_LOCK
    size_t size = GET_SIZE(hdr);
    void *ftr = PTR_ADD(hdr, size - WSIZE);
    PUT(hdr, size);
    PUT(ftr, size);
    coalesce(ptr);
    g_heap.alloc -= size;
    HEAP_UNLOCK
}

void *calloc(size_t nmemb, size_t size) {
    trace("calloc(%zu, %zu)\n", nmemb, size);
    size_t asize = nmemb * size;
    if (asize / size != nmemb) {
        errno = ENOMEM;
        return NULL;
    }
    void *ptr = malloc(asize);
    if (ptr) {
        memset(ptr, 0, asize);
    }
    return ptr;
}

void *realloc(void *ptr, size_t size) {
    trace("realloc(%p, %zu)\n", ptr, size);
    if (!ptr) {
        return malloc(size);
    }
    size_t bsize = GET_SIZE(PTR_SUB(ptr, WSIZE)) - DSIZE;
    void *newptr = ptr;
    if (bsize < size) {
        if ((newptr = malloc(size))) {
            memcpy(newptr, ptr, bsize);
            free(ptr);
        }
    }
    return newptr;
}

void *reallocarray(void *ptr, size_t nmemb, size_t size) {
    size_t asize = nmemb * size;
    if (asize / size != nmemb) {
        errno = ENOMEM;
        return NULL;
    }
    return realloc(ptr, asize);
}

// This is the implementation of next-fit
// Instead of always starting  search from the beginning of the heap,
// next-fir remembers the last place it stopped and resumes searching from that point
static void *find_fit(size_t size) {
    if (g_heap.head == NULL) {
        heap_init();
    }

    void *start = g_heap.last;
    void *hdr = PTR_SUB(start, WSIZE);
    word_t bdata;
    size_t bsize;

    // Search from last to end of heap
    while ((bsize = (bdata = GET(hdr)) & ~0x7)) {
        if (!(bdata & 1) && bsize >= size) {
            g_heap.last = PTR_ADD(hdr, WSIZE); // it updated last found
            return g_heap.last;
        }
        hdr = PTR_ADD(hdr, bsize);
    }

    // search again
    hdr = PTR_SUB(g_heap.head, WSIZE);
    while (hdr < PTR_SUB(start, WSIZE)) {
        bsize = GET_SIZE(hdr);
        if (!(GET_ALLOC(hdr)) && bsize >= size) {
            g_heap.last = PTR_ADD(hdr, WSIZE);
            return g_heap.last;
        }
        hdr = PTR_ADD(hdr, bsize);
    }

    return NULL; // no fit found
}
