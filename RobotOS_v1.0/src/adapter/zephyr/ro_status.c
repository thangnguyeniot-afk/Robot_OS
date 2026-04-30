/* ============================================================================
 * ro_status.c — Zephyr errno → ro_status_t translation
 * ============================================================================
 * Layer: Adapter / Zephyr backend
 * ========================================================================= */

#include <robotos/ro_status.h>

#ifndef ROBOTOS_HOST_BUILD
#include <zephyr/kernel.h>
#include <errno.h>
#endif

/* ---- Zephyr errno translation -------------------------------------------- */

ro_status_t ro_from_zephyr(int zephyr_errno)
{
    if (zephyr_errno == 0) return RO_OK;

    /* Zephyr returns negative errno — normalize */
    int e = (zephyr_errno < 0) ? -zephyr_errno : zephyr_errno;

    switch (e) {
        case ETIMEDOUT: return RO_ETIMEDOUT;
        case EAGAIN:    return RO_EAGAIN;
        case EBUSY:     return RO_EBUSY;
        case ECANCELED: return RO_ECANCELED;
        case EINVAL:    return RO_EINVAL;
        case ENOMEM:    return RO_ENOMEM;
        case ENODEV:    return RO_ENODEV;
        case ENOTSUP:   return RO_ENOTSUP;
        case EOVERFLOW: return RO_EOVERFLOW;
        default:        return RO_EFAIL;
    }
}

/* ---- Status name lookup -------------------------------------------------- */

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
