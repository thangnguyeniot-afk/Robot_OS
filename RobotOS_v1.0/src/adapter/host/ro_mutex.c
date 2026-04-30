/* ============================================================================
 * ro_mutex.c — Host Mutex Backend
 * ============================================================================
 * Layer: Adapter / Host build
 * ========================================================================= */

#include <robotos/ro_mutex.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <pthread.h>
#endif

#ifdef _WIN32
static inline CRITICAL_SECTION* get_cs(ro_mutex_t* mtx) {
    return (CRITICAL_SECTION*)mtx->_impl;
}
#else
static inline pthread_mutex_t* get_pm(ro_mutex_t* mtx) {
    return (pthread_mutex_t*)mtx->_impl;
}
#endif

ro_status_t ro_mutex_create(ro_mutex_t* mtx)
{
    if (mtx == NULL) return RO_EINVAL;
    memset(mtx, 0, sizeof(*mtx));
#ifdef _WIN32
    InitializeCriticalSection(get_cs(mtx));
#else
    pthread_mutex_init(get_pm(mtx), NULL);
#endif
    return RO_OK;
}

void ro_mutex_destroy(ro_mutex_t* mtx)
{
    if (mtx == NULL) return;
#ifdef _WIN32
    DeleteCriticalSection(get_cs(mtx));
#else
    pthread_mutex_destroy(get_pm(mtx));
#endif
}

ro_status_t ro_mutex_lock(ro_mutex_t* mtx, uint32_t timeout_ms)
{
    if (mtx == NULL) return RO_EINVAL;
    (void)timeout_ms;  /* Host build: always blocking lock */
#ifdef _WIN32
    EnterCriticalSection(get_cs(mtx));
#else
    pthread_mutex_lock(get_pm(mtx));
#endif
    return RO_OK;
}

ro_status_t ro_mutex_unlock(ro_mutex_t* mtx)
{
    if (mtx == NULL) return RO_EINVAL;
#ifdef _WIN32
    LeaveCriticalSection(get_cs(mtx));
#else
    pthread_mutex_unlock(get_pm(mtx));
#endif
    return RO_OK;
}
