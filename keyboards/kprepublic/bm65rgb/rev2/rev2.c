/* Copyright 2021 sigprof 2021 peepeetee
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

#include "rev1.h"

#ifdef RGB_MATRIX_ENABLE
#    include <i2c_master.h>
#    include <is31fl3733.h>
#    include <ws2812.h>

// clang-format off

// Dummy IS31FL3733 config for testing - enables all possible LED locations
const is31_led __flash g_is31_leds[DRIVER_LED_TOTAL] = {
    { 0, A_1, B_1, C_1 },
    { 0, A_2, B_2, C_2 },
    { 0, A_3, B_3, C_3 },
    { 0, A_4, B_4, C_4 },
    { 0, A_5, B_5, C_5 },
    { 0, A_6, B_6, C_6 },
    { 0, A_7, B_7, C_7 },
    { 0, A_8, B_8, C_8 },
    { 0, A_9, B_9, C_9 },
    { 0, A_10, B_10, C_10 },
    { 0, A_11, B_11, C_11 },
    { 0, A_12, B_12, C_12 },
    { 0, A_13, B_13, C_13 },
    { 0, A_14, B_14, C_14 },
    { 0, A_15, B_15, C_15 },
    { 0, A_16, B_16, C_16 },
    { 0, D_1, E_1, F_1 },
    { 0, D_2, E_2, F_2 },
    { 0, D_3, E_3, F_3 },
    { 0, D_4, E_4, F_4 },
    { 0, D_5, E_5, F_5 },
    { 0, D_6, E_6, F_6 },
    { 0, D_7, E_7, F_7 },
    { 0, D_8, E_8, F_8 },
    { 0, D_9, E_9, F_9 },
    { 0, D_10, E_10, F_10 },
    { 0, D_11, E_11, F_11 },
    { 0, D_12, E_12, F_12 },
    { 0, D_13, E_13, F_13 },
    { 0, D_14, E_14, F_14 },
    { 0, D_15, E_15, F_15 },
    { 0, D_16, E_16, F_16 },
    { 0, G_1, H_1, I_1 },
    { 0, G_2, H_2, I_2 },
    { 0, G_3, H_3, I_3 },
    { 0, G_4, H_4, I_4 },
    { 0, G_5, H_5, I_5 },
    { 0, G_6, H_6, I_6 },
    { 0, G_7, H_7, I_7 },
    { 0, G_8, H_8, I_8 },
    { 0, G_9, H_9, I_9 },
    { 0, G_10, H_10, I_10 },
    { 0, G_11, H_11, I_11 },
    { 0, G_12, H_12, I_12 },
    { 0, G_13, H_13, I_13 },
    { 0, G_14, H_14, I_14 },
    { 0, G_15, H_15, I_15 },
    { 0, G_16, H_16, I_16 },
    { 0, J_1, K_1, L_1 },
    { 0, J_2, K_2, L_2 },
    { 0, J_3, K_3, L_3 },
    { 0, J_4, K_4, L_4 },
    { 0, J_5, K_5, L_5 },
    { 0, J_6, K_6, L_6 },
    { 0, J_7, K_7, L_7 },
    { 0, J_8, K_8, L_8 },
    { 0, J_9, K_9, L_9 },
    { 0, J_10, K_10, L_10 },
    { 0, J_11, K_11, L_11 },
    { 0, J_12, K_12, L_12 },
    { 0, J_13, K_13, L_13 },
    { 0, J_14, K_14, L_14 },
    { 0, J_15, K_15, L_15 },
    { 0, J_16, K_16, L_16 },
};

// Dummy RGB Matrix config for testing
led_config_t g_led_config = {
    // Matrix location to LED index
    {
        [0 ... MATRIX_ROWS-1] = {
            [0 ... MATRIX_COLS-1] = NO_LED
        }
    },

    // LED index to physical location
    {
        [0 ... DRIVER_LED_TOTAL-1] = {0, 0}
    },

    // LED index to flags
    {
        [0 ... DRIVER_LED_TOTAL-1] = 4
    }
};
// clang-format on

// ==========================================================================
// Custom RGB Matrix driver that combines IS31FL3733 and WS2812
// ==========================================================================
#    if WS2812_LED_TOTAL > 0
LED_TYPE rgb_matrix_ws2812_array[WS2812_LED_TOTAL];
#    endif
static void rgb_matrix_driver_init(void) {
    i2c_init();
    IS31FL3733_init(DRIVER_ADDR_1, 0);
    for (uint8_t index = 0; index < ISSI_LED_TOTAL; index++) {
        bool enabled = true;
        IS31FL3733_set_led_control_register(index, enabled, enabled, enabled);
    }
    IS31FL3733_update_led_control_registers(DRIVER_ADDR_1, 0);
}
static void rgb_matrix_driver_flush(void) {
    IS31FL3733_update_pwm_buffers(DRIVER_ADDR_1, 0);
#    if WS2812_LED_TOTAL > 0
    ws2812_setleds(rgb_matrix_ws2812_array, WS2812_LED_TOTAL);
#    endif
}
static void rgb_matrix_driver_set_color(int index, uint8_t red, uint8_t green, uint8_t blue) {
    if (index < ISSI_LED_TOTAL) {
        IS31FL3733_set_color(index, red, green, blue);
    } else {
#    if WS2812_LED_TOTAL > 0
        rgb_matrix_ws2812_array[index - ISSI_LED_TOTAL].r = red;
        rgb_matrix_ws2812_array[index - ISSI_LED_TOTAL].g = green;
        rgb_matrix_ws2812_array[index - ISSI_LED_TOTAL].b = blue;
#    endif
    }
}
static void rgb_matrix_driver_set_color_all(uint8_t red, uint8_t green, uint8_t blue) {
    IS31FL3733_set_color_all(red, green, blue);
#    if WS2812_LED_TOTAL > 0
    for (uint8_t i = 0; i < WS2812_LED_TOTAL; i++) {
        rgb_matrix_ws2812_array[i].r = red;
        rgb_matrix_ws2812_array[i].g = green;
        rgb_matrix_ws2812_array[i].b = blue;
    }
#    endif
}
// clang-format off
const rgb_matrix_driver_t rgb_matrix_driver = {
    .init          = rgb_matrix_driver_init,
    .flush         = rgb_matrix_driver_flush,
    .set_color     = rgb_matrix_driver_set_color,
    .set_color_all = rgb_matrix_driver_set_color_all,
};
// clang-format on
#endif /* RGB_MATRIX_ENABLE */
