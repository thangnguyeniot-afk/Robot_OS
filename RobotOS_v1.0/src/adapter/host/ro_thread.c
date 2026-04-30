/* ============================================================================
 * ro_thread.c — Host Thread Backend (Windows / POSIX)
 * ============================================================================
 * Layer: Adapter / Host build
 *
 * For unit tests, threads are simplified: create() stores metadata but
 * does NOT spawn a real OS thread unless the test explicitly starts it.
 * Host tests are typically single-threaded — they call functions directly.
 * ========================================================================= */

#include <robotos/ro_thread.h>
#include <robotos/ro_assert.h>

#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <pthread.h>
    #include <unistd.h>
#endif

#ifndef CONFIG_RO_MAX_THREADS
#define CONFIG_RO_MAX_THREADS 8
#endif

typedef struct ro_thread {
    bool                in_use;
    const char*         name;
    ro_thread_entry_t   entry;
    void*               arg;
#ifdef _WIN32
    HANDLE              handle;
#else
    pthread_t           pthread;
#endif
    bool                started;
} ro_thread_slot_t;

static ro_thread_slot_t s_slots[CONFIG_RO_MAX_THREADS];

/* ---- Platform thread entry ---------------------------------------------- */

#ifdef _WIN32
static DWORD WINAPI win_thread_entry(LPVOID param)
{
    ro_thread_slot_t* slot = (ro_thread_slot_t*)param;
    if (slot->entry) slot->entry(slot->arg);
    return 0;
}
#else
static void* posix_thread_entry(void* param)
{
    ro_thread_slot_t* slot = (ro_thread_slot_t*)param;
    if (slot->entry) slot->entry(slot->arg);
    return NULL;
}
#endif

/* ---- API ----------------------------------------------------------------- */

ro_thread_t* ro_thread_create(const ro_thread_config_t* cfg)
{
    RO_ASSERT(cfg != NULL, "ro_thread_create: NULL config");
    RO_ASSERT(cfg->entry != NULL, "ro_thread_create: NULL entry");

    for (int i = 0; i < CONFIG_RO_MAX_THREADS; i++) {
        if (!s_slots[i].in_use) {
            ro_thread_slot_t* slot = &s_slots[i];
            memset(slot, 0, sizeof(*slot));
            slot->in_use = true;
            slot->name   = cfg->name ? cfg->name : "unnamed";
            slot->entry  = cfg->entry;
            slot->arg    = cfg->arg;

#ifdef _WIN32
            slot->handle = CreateThread(NULL, cfg->stack_size,
                                         win_thread_entry, slot, 0, NULL);
            slot->started = (slot->handle != NULL);
#else
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            if (cfg->stack_size > 0) {
                pthread_attr_setstacksize(&attr, cfg->stack_size);
            }
            int ret = pthread_create(&slot->pthread, &attr,
                                      posix_thread_entry, slot);
            pthread_attr_destroy(&attr);
            slot->started = (ret == 0);
#endif
            return (ro_thread_t*)slot;
        }
    }
    return NULL;
}

void ro_thread_destroy(ro_thread_t* t)
{
    if (t == NULL) return;
    ro_thread_slot_t* slot = (ro_thread_slot_t*)t;

#ifdef _WIN32
    if (slot->handle) {
        TerminateThread(slot->handle, 0);
        CloseHandle(slot->handle);
    }
#else
    if (slot->started) {
        pthread_cancel(slot->pthread);
        pthread_join(slot->pthread, NULL);
    }
#endif
    memset(slot, 0, sizeof(*slot));
}

void ro_thread_sleep_ms(uint32_t ms)
{
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000u);
#endif
}

void ro_thread_yield(void)
{
#ifdef _WIN32
    SwitchToThread();
#else
    sched_yield();
#endif
}

const char* ro_thread_get_name(const ro_thread_t* t)
{
    if (t == NULL) return "(null)";
    return ((const ro_thread_slot_t*)t)->name;
}
