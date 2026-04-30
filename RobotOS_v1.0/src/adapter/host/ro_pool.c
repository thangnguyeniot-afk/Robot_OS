/* ============================================================================
 * ro_pool.c — Host Pool Backend (reuses bitmap logic — no Zephyr deps)
 * ============================================================================
 * Layer: Adapter / Host build
 *
 * This is the same bitmap allocator as the Zephyr version — the algorithm
 * has zero Zephyr dependencies.  The only difference is the lock backend:
 *   MUTEX mode uses pthread_mutex / CRITICAL_SECTION instead of k_mutex.
 * ========================================================================= */

#include <robotos/ro_pool.h>
#include <robotos/ro_assert.h>
#include <string.h>

/* ---- Lock helpers (host) ------------------------------------------------- */

/* For simplicity in host builds, we use no actual locking — unit tests
 * are single-threaded.  A production host build would use pthread_mutex. */

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

    /* All blocks free: bit = 1 */
    for (uint32_t i = 0; i < pool->bitmap_words; i++) {
        pool->bitmap[i] = 0xFFFFFFFF;
    }
    uint32_t remainder = cfg->num_blocks % 32;
    if (remainder > 0) {
        pool->bitmap[pool->bitmap_words - 1] = (1u << remainder) - 1;
    }

    memset(&pool->stats, 0, sizeof(pool->stats));
    pool->stats.total = cfg->num_blocks;

    return RO_OK;
}

/* ---- Alloc / Free -------------------------------------------------------- */

static void* pool_alloc_internal(ro_pool_t* pool)
{
    for (uint32_t w = 0; w < pool->bitmap_words; w++) {
        uint32_t word = pool->bitmap[w];
        if (word == 0) continue;

        int bit = __builtin_ctz(word);
        uint32_t block_idx = w * 32 + (uint32_t)bit;
        if (block_idx >= pool->num_blocks) break;

        pool->bitmap[w] &= ~(1u << bit);
        pool->stats.used++;
        if (pool->stats.used > pool->stats.peak) {
            pool->stats.peak = pool->stats.used;
        }
        return (uint8_t*)pool->buffer + block_idx * pool->block_size;
    }

    pool->stats.failures++;
    return NULL;
}

static void pool_free_internal(ro_pool_t* pool, void* block)
{
    if (block == NULL) return;
    uintptr_t offset = (uintptr_t)block - (uintptr_t)pool->buffer;
    uint32_t block_idx = (uint32_t)(offset / pool->block_size);

    RO_ASSERT(block_idx < pool->num_blocks, "ro_pool_free: invalid block");
    RO_ASSERT(offset % pool->block_size == 0, "ro_pool_free: misaligned");

    uint32_t word_idx = block_idx / 32;
    uint32_t bit_idx  = block_idx % 32;

    RO_ASSERT(!(pool->bitmap[word_idx] & (1u << bit_idx)), "double-free");

    pool->bitmap[word_idx] |= (1u << bit_idx);
    pool->stats.used--;
}

void* ro_pool_alloc(ro_pool_t* pool)     { return pool_alloc_internal(pool); }
void* ro_pool_alloc_isr(ro_pool_t* pool) { return pool_alloc_internal(pool); }
void  ro_pool_free(ro_pool_t* pool, void* b)     { pool_free_internal(pool, b); }
void  ro_pool_free_isr(ro_pool_t* pool, void* b) { pool_free_internal(pool, b); }

void ro_pool_get_stats(const ro_pool_t* pool, ro_pool_stats_t* s)
{
    if (pool && s) *s = pool->stats;
}
