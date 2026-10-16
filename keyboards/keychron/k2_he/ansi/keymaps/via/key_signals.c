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

#include QMK_KEYBOARD_H
#include "via.h"
#include "key_signals.h"

#define KEY_SIGNAL_BLINK_PERIOD 1000
#define KEY_SIGNAL_PULSE_PERIOD 1500
#define KEY_SIGNAL_PULSE_FLOOR 40
/* Keep signals readable even when the backlight is dimmed */
#define KEY_SIGNAL_MIN_VAL 96

typedef struct {
    uint8_t  level;
    uint8_t  pattern;
    rgb_t    color;
    uint32_t expires_at; // 0 = never
} key_signal_t;

static key_signal_t signals[RGB_MATRIX_LED_COUNT];

static uint8_t key_signal_set(const uint8_t *args) {
    uint8_t  row     = args[0];
    uint8_t  col     = args[1];
    uint8_t  level   = args[2];
    uint8_t  pattern = args[3];
    uint16_t ttl     = (args[4] << 8) | args[5];

    if (row >= MATRIX_ROWS || col >= MATRIX_COLS) return KEY_SIGNAL_STATUS_BAD_KEY;
    uint8_t led = g_led_config.matrix_co[row][col];
    if (led == NO_LED) return KEY_SIGNAL_STATUS_BAD_KEY;
    if (level > KEY_SIGNAL_CUSTOM || pattern > KEY_SIGNAL_PULSE) return KEY_SIGNAL_STATUS_BAD_VALUE;

    key_signal_t *s = &signals[led];
    s->level        = level;
    s->pattern      = pattern;
    s->expires_at   = ttl ? (timer_read32() + (uint32_t)ttl * 1000) | 1 : 0;

    switch (level) {
        case KEY_SIGNAL_GOOD:
            s->color = (rgb_t){.r = 0, .g = 255, .b = 0};
            break;
        case KEY_SIGNAL_WARN:
            s->color = (rgb_t){.r = 255, .g = 170, .b = 0};
            break;
        case KEY_SIGNAL_ALERT:
            s->color = (rgb_t){.r = 255, .g = 0, .b = 0};
            break;
        case KEY_SIGNAL_CUSTOM:
            s->color = (rgb_t){.r = args[6], .g = args[7], .b = args[8]};
            break;
    }
    return KEY_SIGNAL_STATUS_OK;
}

void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    uint8_t *command_id = &data[0];
    uint8_t  channel_id = data[1];
    uint8_t  command    = data[2];
    uint8_t *status     = &data[3];
    uint8_t *args       = &data[4];

    if (channel_id != id_custom_channel) {
        *command_id = id_unhandled;
        return;
    }

    switch (command) {
        case KEY_SIGNAL_CMD_SET:
            *status = key_signal_set(args);
            break;
        case KEY_SIGNAL_CMD_CLEAR_ALL:
            memset(signals, 0, sizeof(signals));
            *status = KEY_SIGNAL_STATUS_OK;
            break;
        case KEY_SIGNAL_CMD_INFO:
            args[0] = KEY_SIGNAL_PROTOCOL_VERSION;
            args[1] = MATRIX_ROWS;
            args[2] = MATRIX_COLS;
            *status = KEY_SIGNAL_STATUS_OK;
            break;
        default:
            *status = KEY_SIGNAL_STATUS_BAD_COMMAND;
            break;
    }
}

static uint8_t pattern_scale(uint8_t pattern, uint32_t now) {
    switch (pattern) {
        case KEY_SIGNAL_BLINK:
            return (now % KEY_SIGNAL_BLINK_PERIOD) < KEY_SIGNAL_BLINK_PERIOD / 2 ? 255 : 0;
        case KEY_SIGNAL_PULSE: {
            uint32_t half  = KEY_SIGNAL_PULSE_PERIOD / 2;
            uint32_t phase = now % KEY_SIGNAL_PULSE_PERIOD;
            uint32_t ramp  = phase < half ? phase : KEY_SIGNAL_PULSE_PERIOD - phase;
            return KEY_SIGNAL_PULSE_FLOOR + ramp * (255 - KEY_SIGNAL_PULSE_FLOOR) / half;
        }
        default:
            return 255;
    }
}

void key_signals_render(uint8_t led_min, uint8_t led_max) {
    uint32_t now = timer_read32();
    uint8_t  val = MAX(rgb_matrix_get_val(), KEY_SIGNAL_MIN_VAL);

    for (uint8_t i = led_min; i < led_max; i++) {
        key_signal_t *s = &signals[i];
        if (s->level == KEY_SIGNAL_OFF) continue;
        if (s->expires_at && timer_expired32(now, s->expires_at)) {
            s->level = KEY_SIGNAL_OFF;
            continue;
        }
        uint16_t scale = (uint16_t)val * pattern_scale(s->pattern, now) / 255;
        rgb_matrix_set_color(i, s->color.r * scale / 255, s->color.g * scale / 255, s->color.b * scale / 255);
    }
}
