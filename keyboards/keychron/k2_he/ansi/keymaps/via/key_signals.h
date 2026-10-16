/* Copyright 2026 Bjonnh
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Key signals: host programs light individual keys over raw HID.
 *
 * Wire compatible with the keysignal host tool from
 * https://github.com/heydaytime/k2he (host/). Packets are 32 bytes on the
 * VIA custom value channel 0, which neither VIA nor Keychron use:
 *
 *   [0] 0x07 (id_custom_set_value) or 0x08 (id_custom_get_value)
 *   [1] 0x00 (id_custom_channel)
 *   [2] command, KEY_SIGNAL_CMD_*
 *   [3] status, filled in by the keyboard in its echoed reply
 *   [4..] arguments:
 *     SET:       row, col, level, pattern, ttl_hi, ttl_lo (seconds, 0 = forever), r, g, b
 *     CLEAR_ALL: none
 *     INFO:      reply is protocol version, MATRIX_ROWS, MATRIX_COLS
 *
 * State lives in RAM only and is drawn over the current RGB effect.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#define KEY_SIGNAL_PROTOCOL_VERSION 1

enum key_signal_command {
    KEY_SIGNAL_CMD_SET       = 0x01,
    KEY_SIGNAL_CMD_CLEAR_ALL = 0x02,
    KEY_SIGNAL_CMD_INFO      = 0x03,
};

enum key_signal_status {
    KEY_SIGNAL_STATUS_OK          = 0x00,
    KEY_SIGNAL_STATUS_BAD_COMMAND = 0x01,
    KEY_SIGNAL_STATUS_BAD_KEY     = 0x02,
    KEY_SIGNAL_STATUS_BAD_VALUE   = 0x03,
};

enum key_signal_level {
    KEY_SIGNAL_OFF,
    KEY_SIGNAL_GOOD,
    KEY_SIGNAL_WARN,
    KEY_SIGNAL_ALERT,
    KEY_SIGNAL_CUSTOM,
};

enum key_signal_pattern {
    KEY_SIGNAL_SOLID,
    KEY_SIGNAL_BLINK,
    KEY_SIGNAL_PULSE,
};

/* Call from rgb_matrix_indicators_advanced_user() */
void key_signals_render(uint8_t led_min, uint8_t led_max);
