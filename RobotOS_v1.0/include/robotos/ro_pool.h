/* ============================================================================
 * ro_pool.h — RobotOS Fixed-Block Memory Pool
 * ============================================================================
 * Layer: Adapter (public header — NO <zephyr/*> includes)
 *
 * Programming technique: Bitmap-based fixed-block allocator.
 *
 * Data structure:
 *   - A contiguous static buffer is divided into N blocks of equal size.
 *   - A bitmap (uint32_t array) tracks which blocks are free (1) / used (0).
 *   - alloc() scans the bitmap for the first set bit, clears it, returns ptr.
 *   - free() sets the corresponding bit back.
 *
 * Three lock modes:
 *   RO_POOL_LOCK_NONE   — no synchronization (single-threaded / ISR-only)
 *   RO_POOL_LOCK_ATOMIC — lock-free via compare-and-swap on bitmap words
 *   RO_POOL_LOCK_MUTEX  — mutex-guarded (priority inheritance on Zephyr)
 *
 * Memory discipline: NO malloc.  The caller provides the backing buffer
 * and bitmap storage at compile time.
 * ========================================================================= */

#ifndef ROBOTOS_RO_POOL_H
#define ROBOTOS_RO_POOL_H

#include <robotos/ro_status.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Lock mode ----------------------------------------------------------- */

typedef enum {
    RO_POOL_LOCK_NONE   = 0,  /* No synchronization                    */
    RO_POOL_LOCK_ATOMIC = 1,  /* CAS-based (ISR-safe + thread-safe)    */
    RO_POOL_LOCK_MUTEX  = 2,  /* Mutex with priority inheritance       */
} ro_pool_lock_t;

/* ---- Pool configuration -------------------------------------------------- */

/**
 * ro_pool_config_t — Static pool descriptor.
 *
 * Caller fills this structure with pointers to pre-allocated memory
 * and passes it to ro_pool_init().
 */
typedef struct {
    void*          buffer;      /* Pre-allocated block array             */
    uint32_t       block_size;  /* Size of each block in bytes (> 0)    */
    uint32_t       num_blocks;  /* Total number of blocks (> 0)         */
    ro_pool_lock_t lock_mode;   /* Synchronization strategy             */
} ro_pool_config_t;

/* ---- Pool statistics ----------------------------------------------------- */

typedef struct {
    uint32_t total;     /* Total blocks in pool                  */
    uint32_t used;      /* Currently allocated blocks            */
    uint32_t peak;      /* High-water mark of `used`             */
    uint32_t failures;  /* Allocation attempts when pool was full */
} ro_pool_stats_t;

/* ---- Bitmap helper —— compute bitmap array size at compile time ---------- */

/**
 * RO_POOL_BITMAP_SIZE(num_blocks)
 *
 * Returns the number of uint32_t words needed to bitmap `num_blocks` slots.
 * Use this to size the static bitmap array.
 *
 * Example:
 *   static uint32_t my_bitmap[RO_POOL_BITMAP_SIZE(16)];
 */
#define RO_POOL_BITMAP_SIZE(n) (((n) + 31u) / 32u)

/* ---- Pool handle --------------------------------------------------------- */

typedef struct {
    void*          buffer;       /* Block array base address             */
    uint32_t       block_size;   /* Bytes per block                     */
    uint32_t       num_blocks;   /* Total blocks                        */
    uint32_t*      bitmap;       /* Bitmap array (external storage)     */
    uint32_t       bitmap_words; /* Number of uint32_t words in bitmap  */
    ro_pool_lock_t lock_mode;    /* Selected lock strategy              */
    ro_pool_stats_t stats;       /* Live statistics                     */
    uint8_t        _impl[32];   /* Backend lock state (opaque)         */
} ro_pool_t;

/* ---- Lifecycle ----------------------------------------------------------- */

/**
 * Initialize a pool from caller-provided storage.
 *
 * @param pool    Pool object
 * @param cfg     Configuration (buffer, block_size, num_blocks, lock_mode)
 * @param bitmap  Caller-provided bitmap array of RO_POOL_BITMAP_SIZE(num_blocks)
 *
 * Returns: RO_OK, RO_EINVAL on bad parameters.
 */
ro_status_t ro_pool_init(ro_pool_t* pool,
                         const ro_pool_config_t* cfg,
                         uint32_t* bitmap);

/* ---- Thread-safe alloc/free ---------------------------------------------- */

/**
 * Allocate one block from the pool.
 * Returns pointer to the block, or NULL if pool exhausted.
 */
void* ro_pool_alloc(ro_pool_t* pool);

/**
 * Free a block back to the pool.
 * `block` MUST have been returned by ro_pool_alloc on the same pool.
 */
void ro_pool_free(ro_pool_t* pool, void* block);

/* ---- ISR-safe alloc/free ------------------------------------------------- */

/** Allocate from ISR context. Only valid if lock_mode is NONE or ATOMIC. */
void* ro_pool_alloc_isr(ro_pool_t* pool);

/** Free from ISR context. Only valid if lock_mode is NONE or ATOMIC. */
void  ro_pool_free_isr(ro_pool_t* pool, void* block);

/* ---- Inspection ---------------------------------------------------------- */

/** Copy current statistics into `stats_out`. Thread-safe snapshot. */
void ro_pool_get_stats(const ro_pool_t* pool, ro_pool_stats_t* stats_out);

#ifdef __cplusplus
}
#endif

#endif /* ROBOTOS_RO_POOL_H */
