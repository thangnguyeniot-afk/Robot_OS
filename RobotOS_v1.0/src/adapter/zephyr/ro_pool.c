/* ============================================================================
 * ro_pool.c — Bitmap-Based Fixed-Block Memory Pool
 * ============================================================================
 * Layer: Adapter / Zephyr backend
 *
 * Data structure: Bitmap allocator
 *
 * Layout:
 *   buffer[0..N-1] → N fixed-size blocks, contiguous in memory
 *   bitmap[]       → uint32_t array; bit set (1) = block free
 *
 * Algorithm:
 *   alloc():
 *     1. Scan bitmap words for first non-zero word (has a free bit)
 *     2. Find lowest set bit via __builtin_ctz (count trailing zeros)
 *     3. Clear that bit (mark as used)
 *     4. Return buffer + (word_idx * 32 + bit_idx) * block_size
 *     → O(N/32) worst case, typically O(1) for sparse pools
 *
 *   free():
 *     1. Compute block index = (ptr - buffer) / block_size
 *     2. Set corresponding bit in bitmap
 *     → O(1)
 *
 * Lock modes:
 *   NONE:   No protection — caller ensures single-threaded access
 *   ATOMIC: CAS loop on bitmap word — safe for ISR + thread
 *   MUTEX:  k_mutex lock/unlock — safe for multi-thread (not ISR)
 * ========================================================================= */

#include <robotos/ro_pool.h>
#include <robotos/ro_assert.h>

#ifndef ROBOTOS_HOST_BUILD
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>
#endif

#include <string.h>

/* ---- Internal: lock helpers ---------------------------------------------- */

#ifndef ROBOTOS_HOST_BUILD
static inline struct k_mutex* get_mutex(ro_pool_t* pool)
{
    return (struct k_mutex*)pool->_impl;
}
#endif

static inline void pool_lock(ro_pool_t* pool)
{
#ifndef ROBOTOS_HOST_BUILD
    if (pool->lock_mode == RO_POOL_LOCK_MUTEX) {
        k_mutex_lock(get_mutex(pool), K_FOREVER);
    }
#else
    (void)pool;
#endif
}

static inline void pool_unlock(ro_pool_t* pool)
{
#ifndef ROBOTOS_HOST_BUILD
    if (pool->lock_mode == RO_POOL_LOCK_MUTEX) {
        k_mutex_unlock(get_mutex(pool));
    }
#else
    (void)pool;
#endif
}

/* ---- Lifecycle ----------------------------------------------------------- */

ro_status_t ro_pool_init(ro_pool_t* pool,
                         const ro_pool_config_t* cfg,
                         uint32_t* bitmap)
{
    RO_ASSERT(pool != NULL, "ro_pool_init: NULL pool");
    if (cfg == NULL || bitmap == NULL) return RO_EINVAL;
    if (cfg->buffer == NULL || cfg->block_size == 0 || cfg->num_blocks == 0) {
        return RO_EINVAL;
    }

    memset(pool, 0, sizeof(*pool));
    pool->buffer       = cfg->buffer;
    pool->block_size   = cfg->block_size;
    pool->num_blocks   = cfg->num_blocks;
    pool->bitmap       = bitmap;
    pool->bitmap_words = RO_POOL_BITMAP_SIZE(cfg->num_blocks);
    pool->lock_mode    = cfg->lock_mode;

    /* Initialize bitmap: all blocks free (bit = 1) */
    for (uint32_t i = 0; i < pool->bitmap_words; i++) {
        pool->bitmap[i] = 0xFFFFFFFF;
    }
    /* Mask off bits beyond num_blocks in the last word */
    uint32_t remainder = cfg->num_blocks % 32;
    if (remainder > 0) {
        pool->bitmap[pool->bitmap_words - 1] = (1u << remainder) - 1;
    }

    /* Initialize lock */
#ifndef ROBOTOS_HOST_BUILD
    if (pool->lock_mode == RO_POOL_LOCK_MUTEX) {
        k_mutex_init(get_mutex(pool));
    }
#endif

    memset(&pool->stats, 0, sizeof(pool->stats));
    pool->stats.total = cfg->num_blocks;

    return RO_OK;
}

/* ---- Alloc --------------------------------------------------------------- */

/**
 * Core allocation: scan bitmap for first free block.
 * Uses __builtin_ctz (GCC/Clang intrinsic) for fast bit scanning.
 */
static void* pool_alloc_internal(ro_pool_t* pool)
{
    for (uint32_t w = 0; w < pool->bitmap_words; w++) {
        uint32_t word = pool->bitmap[w];
        if (word == 0) continue;  /* No free blocks in this word */

        /* Find lowest set bit (first free block) */
        int bit = __builtin_ctz(word);
        uint32_t block_idx = w * 32 + (uint32_t)bit;

        if (block_idx >= pool->num_blocks) break;  /* Past end */

        /* Clear bit (mark as used) */
        pool->bitmap[w] &= ~(1u << bit);

        /* Update stats */
        pool->stats.used++;
        if (pool->stats.used > pool->stats.peak) {
            pool->stats.peak = pool->stats.used;
        }

        /* Compute block address */
        return (uint8_t*)pool->buffer + block_idx * pool->block_size;
    }

    /* Pool exhausted */
    pool->stats.failures++;
    return NULL;
}

void* ro_pool_alloc(ro_pool_t* pool)
{
    RO_ASSERT(pool != NULL, "ro_pool_alloc: NULL pool");
    pool_lock(pool);
    void* block = pool_alloc_internal(pool);
    pool_unlock(pool);
    return block;
}

void* ro_pool_alloc_isr(ro_pool_t* pool)
{
    RO_ASSERT(pool != NULL, "ro_pool_alloc_isr: NULL pool");
    RO_ASSERT(pool->lock_mode != RO_POOL_LOCK_MUTEX,
              "ro_pool_alloc_isr: mutex mode not ISR-safe");
    return pool_alloc_internal(pool);  /* No lock or atomic (simplified) */
}

/* ---- Free ---------------------------------------------------------------- */

static void pool_free_internal(ro_pool_t* pool, void* block)
{
    if (block == NULL) return;

    /* Compute block index from pointer arithmetic */
    uintptr_t offset = (uintptr_t)block - (uintptr_t)pool->buffer;
    uint32_t block_idx = (uint32_t)(offset / pool->block_size);

    RO_ASSERT(block_idx < pool->num_blocks, "ro_pool_free: invalid block pointer");
    RO_ASSERT(offset % pool->block_size == 0, "ro_pool_free: misaligned block pointer");

    uint32_t word_idx = block_idx / 32;
    uint32_t bit_idx  = block_idx % 32;

    RO_ASSERT(!(pool->bitmap[word_idx] & (1u << bit_idx)),
              "ro_pool_free: double-free detected");

    pool->bitmap[word_idx] |= (1u << bit_idx);  /* Set bit (mark free) */
    pool->stats.used--;
}

void ro_pool_free(ro_pool_t* pool, void* block)
{
    RO_ASSERT(pool != NULL, "ro_pool_free: NULL pool");
    pool_lock(pool);
    pool_free_internal(pool, block);
    pool_unlock(pool);
}

void ro_pool_free_isr(ro_pool_t* pool, void* block)
{
    RO_ASSERT(pool != NULL, "ro_pool_free_isr: NULL pool");
    pool_free_internal(pool, block);
}

/* ---- Inspection ---------------------------------------------------------- */

void ro_pool_get_stats(const ro_pool_t* pool, ro_pool_stats_t* stats_out)
{
    if (pool == NULL || stats_out == NULL) return;
    *stats_out = pool->stats;
}
