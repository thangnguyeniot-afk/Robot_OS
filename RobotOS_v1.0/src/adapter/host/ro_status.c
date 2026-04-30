/* ============================================================================
 * ro_status.c — Host Status Backend (reuse — no Zephyr deps)
 * ============================================================================
 * Layer: Adapter / Host build
 * ========================================================================= */

#include <robotos/ro_status.h>
#include <errno.h>

ro_status_t ro_from_zephyr(int zephyr_errno)
{
    /* On host build, this function is rarely called.
     * Map standard POSIX errno values to ro_status_t. */
    if (zephyr_errno == 0) return RO_OK;
    int e = (zephyr_errno < 0) ? -zephyr_errno : zephyr_errno;

    switch (e) {
        case ETIMEDOUT: return RO_ETIMEDOUT;
#ifdef EAGAIN
        case EAGAIN:    return RO_EAGAIN;
#endif
        case EBUSY:     return RO_EBUSY;
#ifdef ECANCELED
        case ECANCELED: return RO_ECANCELED;
#endif
        case EINVAL:    return RO_EINVAL;
        case ENOMEM:    return RO_ENOMEM;
#ifdef ENODEV
        case ENODEV:    return RO_ENODEV;
#endif
#ifdef ENOTSUP
        case ENOTSUP:   return RO_ENOTSUP;
#endif
#ifdef EOVERFLOW
        case EOVERFLOW: return RO_EOVERFLOW;
#endif
        default:        return RO_EFAIL;
    }
}

const char* ro_status_name(ro_status_t s)
{
    switch (s) {
        case RO_OK:         return "RO_OK";
        case RO_ETIMEDOUT:  return "RO_ETIMEDOUT";
        case RO_EAGAIN:     return "RO_EAGAIN";
        case RO_EBUSY:      return "RO_EBUSY";
        case RO_ECANCELED:  return "RO_ECANCELED";
        case RO_EINVAL:     return "RO_EINVAL";
        case RO_ENOMEM:     return "RO_ENOMEM";
        case RO_ENODEV:     return "RO_ENODEV";
        case RO_ENOTSUP:    return "RO_ENOTSUP";
        case RO_EOVERFLOW:  return "RO_EOVERFLOW";
        case RO_EFAIL:      return "RO_EFAIL";
        case RO_ENOTREADY:  return "RO_ENOTREADY";
        case RO_EHWINIT:    return "RO_EHWINIT";
        case RO_EBOOTSEQ:   return "RO_EBOOTSEQ";
        default:            return "RO_UNKNOWN";
    }
}
