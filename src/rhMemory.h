#ifndef RH_MEMORY_H
#define RH_MEMORY_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define TA_BLOCK_COUNT 256    // Increase block count
#define TA_HEAP_SIZE (8 << 20) // 8MB heap

bool ta_init(const void *base, const void *limit, const size_t heap_blocks, const size_t split_thresh, const size_t alignment);
void *ta_alloc(size_t num);
void *ta_calloc(size_t num, size_t size);
bool ta_free(void *ptr);

static uint8_t heap_memory[TA_HEAP_SIZE] __attribute__((aligned(16)));
static bool heap_initialized = false;

void heap_init(void)
{
    ta_init(heap_memory, 
            (uint8_t *)heap_memory + TA_HEAP_SIZE,
            TA_BLOCK_COUNT, 
            16,
            16);
    heap_initialized = true;
}

void *memcpy(void *restrict dest, const void *restrict src, size_t n)
{
    uint8_t *restrict pdest = (uint8_t *restrict)dest;
    const uint8_t *restrict psrc = (const uint8_t *restrict)src;
    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }
    return dest;
}

void *memset(void *s, int c, size_t n)
{
    uint8_t *p = (uint8_t *)s;
    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }
    return s;
}

void *memmove(void *dest, const void *src, size_t n)
{
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;
    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }
    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;
    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }
    return 0;
}

// Magic number to verify our allocations
#define ALLOC_MAGIC 0xDEADBEEF

typedef struct {
    uint32_t magic;
    size_t size;
} alloc_header_t;

#define HEADER_SIZE (sizeof(alloc_header_t))

// Check if pointer is within our heap
static inline bool is_in_heap(void *ptr)
{
    return (uint8_t*)ptr >= heap_memory && 
           (uint8_t*)ptr < (heap_memory + TA_HEAP_SIZE);
}

void *malloc(size_t size)
{
    if (size == 0) return NULL;
    if (! heap_initialized) return NULL;
    
    void *raw = ta_alloc(size + HEADER_SIZE);
    if (raw == NULL) return NULL;
    
    alloc_header_t *header = (alloc_header_t *)raw;
    header->magic = ALLOC_MAGIC;
    header->size = size;
    
    return (uint8_t *)raw + HEADER_SIZE;
}

void free(void *ptr)
{
    if (ptr == NULL) return;
    if (!heap_initialized) return;
    
    // Check if this is our allocation
    alloc_header_t *header = (alloc_header_t *)((uint8_t *)ptr - HEADER_SIZE);
    
    if (! is_in_heap(header)) {
        return; // Not our memory, don't touch it
    }
    
    if (header->magic != ALLOC_MAGIC) {
        return; // Corrupted or not our allocation
    }
    
    // Clear the magic to detect double-free
    header->magic = 0;
    
    ta_free(header);
}

void *realloc(void *ptr, size_t size)
{
    // realloc(NULL, size) is equivalent to malloc(size)
    if (ptr == NULL) {
        return malloc(size);
    }
    
    // realloc(ptr, 0) is equivalent to free(ptr)
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    
    // Check if this is our allocation
    alloc_header_t *header = (alloc_header_t *)((uint8_t *)ptr - HEADER_SIZE);
    
    if (!is_in_heap(header) || header->magic != ALLOC_MAGIC) {
        // Not our allocation - this shouldn't happen with SSFN
        // but let's handle it gracefully by just allocating new memory
        // We can't copy because we don't know the old size
        return malloc(size);
    }
    
    size_t old_size = header->size;
    
    // If shrinking or same size, just return the same pointer
    if (size <= old_size) {
        header->size = size; // Update size
        return ptr;
    }
    
    // Need to grow - allocate new block
    void *new_ptr = malloc(size);
    if (new_ptr == NULL) {
        return NULL;
    }
    
    // Copy old data
    memcpy(new_ptr, ptr, old_size);
    
    // Free old block
    free(ptr);
    
    return new_ptr;
}

static inline void hcf(void)
{
    for(;;) {
        asm("hlt");
    }
}

#endif // RH_MEMORY_H