/*
 * devkit_app_state.c
 * RobotOS devkit minimal application state machine (Phase 9C) — devkit-local.
 *
 * See devkit_app_state.h for the public contract and architectural intent.
 *
 * Implementation notes:
 *   - All entry points are called from a single thread context: the core
 *     dispatcher thread (via the button/UART producer handlers) and the
 *     devkit runtime thread (via init / periodic log). No locks or volatiles.
 *   - No timers, no allocation, no floating point.
 *   - No Zephyr driver dependency; only Zephyr LOG_INF for the snapshot line
 *     and per-transition diagnostic logs.
 */

#include "devkit_app_state.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(devkit_app, LOG_LEVEL_INF);

static devkit_app_state_t s_state;
static uint32_t           s_transitions;
static uint32_t           s_button_count;
static uint32_t           s_uart_count;
static uint32_t           s_ignored_count;
static devkit_app_src_t   s_last_src;
static uint8_t            s_last_byte;

static const char *state_name(devkit_app_state_t s)
{
	switch (s) {
	case DEVKIT_APP_STATE_IDLE:   return "IDLE";
	case DEVKIT_APP_STATE_ARMED:  return "ARMED";
	case DEVKIT_APP_STATE_ACTIVE: return "ACTIVE";
	default:                      return "UNKNOWN";
	}
}

const char *devkit_app_state_state_name(devkit_app_state_t state)
{
	return state_name(state);
}

static const char *src_name(devkit_app_src_t s)
{
	switch (s) {
	case DEVKIT_APP_SRC_NONE: return "NONE";
	case DEVKIT_APP_SRC_BTN:  return "BTN";
	case DEVKIT_APP_SRC_UART: return "UART";
	default:                  return "UNKNOWN";
	}
}

/*
 * Apply a transition and emit a diagnostic line. Always increments
 * s_transitions when called; callers must skip this on no-op queries.
 */
static void transition(devkit_app_state_t next, devkit_app_src_t src,
		       uint32_t seq_or_count, uint8_t byte_or_zero)
{
	devkit_app_state_t prev = s_state;
	s_state = next;
	s_transitions++;

	if (src == DEVKIT_APP_SRC_BTN) {
		LOG_INF("Phase 9C app transition: %s->%s src=BTN seq=%u total=%u",
			state_name(prev), state_name(next),
			seq_or_count, s_transitions);
	} else if (src == DEVKIT_APP_SRC_UART) {
		LOG_INF("Phase 9C app transition: %s->%s src=UART byte=0x%02x count=%u total=%u",
			state_name(prev), state_name(next),
			(unsigned)byte_or_zero,
			seq_or_count, s_transitions);
	} else {
		LOG_INF("Phase 9C app transition: %s->%s src=%s total=%u",
			state_name(prev), state_name(next),
			src_name(src), s_transitions);
	}
}

void devkit_app_state_init(void)
{
	s_state         = DEVKIT_APP_STATE_IDLE;
	s_transitions   = 0u;
	s_button_count  = 0u;
	s_uart_count    = 0u;
	s_ignored_count = 0u;
	s_last_src      = DEVKIT_APP_SRC_NONE;
	s_last_byte     = 0u;
	LOG_INF("Phase 9C app state init: state=IDLE");
}

void devkit_app_state_on_button(uint32_t seq)
{
	s_button_count++;
	s_last_src = DEVKIT_APP_SRC_BTN;
	/* s_last_byte is preserved (button has no byte payload) */

	devkit_app_state_t next;
	switch (s_state) {
	case DEVKIT_APP_STATE_IDLE:   next = DEVKIT_APP_STATE_ARMED;  break;
	case DEVKIT_APP_STATE_ARMED:  next = DEVKIT_APP_STATE_ACTIVE; break;
	case DEVKIT_APP_STATE_ACTIVE: next = DEVKIT_APP_STATE_IDLE;   break;
	default:                      next = DEVKIT_APP_STATE_IDLE;   break;
	}
	transition(next, DEVKIT_APP_SRC_BTN, seq, 0u);
}

void devkit_app_state_on_uart_byte(uint8_t byte, uint32_t handler_count)
{
	s_uart_count++;
	s_last_src  = DEVKIT_APP_SRC_UART;
	s_last_byte = byte;

	/* Case-insensitive ASCII command set; everything else is ignored. */
	uint8_t c = byte;
	if (c >= 'A' && c <= 'Z') {
		c = (uint8_t)(c + ('a' - 'A'));
	}

	switch (c) {
	case 'a':
		if (s_state != DEVKIT_APP_STATE_ARMED) {
			transition(DEVKIT_APP_STATE_ARMED, DEVKIT_APP_SRC_UART,
				   handler_count, byte);
		} else {
			LOG_INF("Phase 9C app: 'a' ignored (already ARMED)");
			s_ignored_count++;
		}
		break;
	case 's':
		if (s_state != DEVKIT_APP_STATE_ACTIVE) {
			transition(DEVKIT_APP_STATE_ACTIVE, DEVKIT_APP_SRC_UART,
				   handler_count, byte);
		} else {
			LOG_INF("Phase 9C app: 's' ignored (already ACTIVE)");
			s_ignored_count++;
		}
		break;
	case 'r':
		if (s_state != DEVKIT_APP_STATE_IDLE) {
			transition(DEVKIT_APP_STATE_IDLE, DEVKIT_APP_SRC_UART,
				   handler_count, byte);
		} else {
			LOG_INF("Phase 9C app: 'r' ignored (already IDLE)");
			s_ignored_count++;
		}
		break;
	case '?':
		LOG_INF("Phase 9C app query: state=%s transitions=%u",
			state_name(s_state), s_transitions);
		/* '?' is a recognized query, not an ignored command. */
		break;
	case 'v':
		/* Phase 10B-v: build/version query. No state change; not an
		 * ignored command. Response is emitted by the UART TX path. */
		LOG_INF("Phase 10B-v build query: state=%s",
			state_name(s_state));
		break;
	case 'l':
		/* Phase 10B-L: LED physical-effect command. No app-state change;
		 * not an ignored command. The actual GPIO toggle is performed
		 * in the UART TX layer (devkit_uart_producer.c) so this module
		 * stays hardware-free. Case-insensitive: `L` and `l` both land
		 * here via the upcase normalization above. */
		LOG_INF("Phase 10B-L LED command: state=%s",
			state_name(s_state));
		break;
	case 't':
		/* Phase 11D: on-board MEMS accelerometer probe command.
		 * No app-state change; not an ignored command. Adapter-level
		 * probe: the actual sensor read (sensor_sample_fetch +
		 * sensor_channel_get) and the ACC/ERR response formatting are
		 * performed in the UART TX layer (devkit_uart_producer.c) so
		 * this module stays hardware-free. Case-insensitive: `T` and
		 * `t` both land here via the upcase normalization above.
		 * Spec: PHASE_11C_ACCEL_PROBE_SPEC.md §G. */
		LOG_INF("Phase 11D-T accel probe: state=%s",
			state_name(s_state));
		break;
	case 'd':
		/* Phase 10B-d: explicit disarm. ARMED -> IDLE with transition.
		 * IDLE: recognized no-op (not ignored -- distinct from 'r').
		 * ACTIVE: recognized no-op (USER_DECISION_REQUIRED_ACTIVE_DISARM).
		 * 'd' does not replace or alter 'r'. */
		if (s_state == DEVKIT_APP_STATE_ARMED) {
			transition(DEVKIT_APP_STATE_IDLE, DEVKIT_APP_SRC_UART,
				   handler_count, byte);
		} else {
			LOG_INF("Phase 10B-d disarm no-op: state=%s",
				state_name(s_state));
		}
		break;
	default:
		s_ignored_count++;
		break;
	}
}

void devkit_app_state_get_snapshot(devkit_app_state_snapshot_t *out)
{
	if (out == NULL) {
		return;
	}
	out->state         = s_state;
	out->transitions   = s_transitions;
	out->button_count  = s_button_count;
	out->uart_count    = s_uart_count;
	out->ignored_count = s_ignored_count;
	out->last_src      = s_last_src;
	out->last_byte     = s_last_byte;
}

void devkit_app_state_log_snapshot(void)
{
	LOG_INF("ROBOTOS_APP state=%s transitions=%u button=%u uart=%u "
		"ignored=%u last_src=%s last_byte=0x%02x",
		state_name(s_state),
		s_transitions,
		s_button_count,
		s_uart_count,
		s_ignored_count,
		src_name(s_last_src),
		(unsigned)s_last_byte);
}
