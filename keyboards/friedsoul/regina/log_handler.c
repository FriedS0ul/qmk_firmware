#include "quantum.h"
#include "config.h"
#include "eeprom_config.h"

static uint16_t scan_counter = 0;

// Вывод лога кажные N полных сканирований матрицы в консоль
void logger(void) {
    if (runtime_config.console_log_status != 0) {
        if (scan_counter < DEFAULT_CONSOLE_LOG_FREQUENCY) {
            scan_counter++;
        } else {
            switch (runtime_config.console_log_status) {
                case 1:
                    uprintf("\r\n");
                    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                            uprintf("%d, %d Floor: %d Ceiling: %d Actuation: %d Release: %d", col, row, runtime_config.floor_level_per_key[col][row], runtime_config.ceiling_level_per_key[col][row], runtime_config.actuation_level_per_key[col][row], runtime_config.release_level_per_key[col][row]);
                            uprintf("\r\n");
                        }
                    }
                    break;

                case 2:
                    uprintf("\r\n");
                    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                            uprintf("COL %d ROW %d: %u", col, row, log_matrix[col][row]);
                            uprintf("\r\n");
                        }
                    }
                    break;

                case 3:
                    uprintf("\r\n");
                    uprintf("EEPROM CURRENT SIZE: %zu bytes \n", sizeof(eeprom_config));
                    uprintf("\r\n");
                    uprintf("FIRMWARE LEVEL %d\n", eeprom_config.fw_level_number);
                    uprintf("CURRENT OPERATION MODE %d\n", runtime_config.kb_current_operation_mode);
                    uprintf("LOG STATUS %d\n", runtime_config.console_log_status);
                    uprintf("ACTUATION GLOBAL %d\n", runtime_config.actuation_level_global);
                    uprintf("RELEASE GLOBAL %d\n", runtime_config.release_level_global);

                    uprintf("\r\n");
                    break;

                case 4:
                    uprintf("\r\n");
                    uprintf("CURRENT PAIR: %d", runtime_config.socd_pair_current);
                    uprintf("\r\n");
                    uprintf("Slot 0 SOCD key 0: %d, %d\n", runtime_config.socd_pair_0.button_0_pos[0], runtime_config.socd_pair_0.button_0_pos[1]);
                    uprintf("Slot 0 SOCD key 1: %d, %d\n", runtime_config.socd_pair_0.button_1_pos[0], runtime_config.socd_pair_0.button_1_pos[1]);
                    uprintf("Slot 0 SOCD mode: %d\n", runtime_config.socd_pair_0.pair_mode);
                    uprintf("\r\n");
                    uprintf("Slot 1 SOCD key 0: %d, %d\n", runtime_config.socd_pair_1.button_0_pos[0], runtime_config.socd_pair_1.button_0_pos[1]);
                    uprintf("Slot 1 SOCD key 1: %d, %d\n", runtime_config.socd_pair_1.button_1_pos[0], runtime_config.socd_pair_1.button_1_pos[1]);
                    uprintf("Slot 1 SOCD mode: %d\n", runtime_config.socd_pair_1.pair_mode);
                    uprintf("\r\n");
                    uprintf("Slot 2 SOCD key 0: %d, %d\n", runtime_config.socd_pair_2.button_0_pos[0], runtime_config.socd_pair_2.button_0_pos[1]);
                    uprintf("Slot 2 SOCD key 1: %d, %d\n", runtime_config.socd_pair_2.button_1_pos[0], runtime_config.socd_pair_2.button_1_pos[1]);
                    uprintf("Slot 2 SOCD mode: %d\n", runtime_config.socd_pair_2.pair_mode);
                    uprintf("\r\n");
                    break;

                case 5:
                    uprintf("\r\n");
                    uprintf("PAIR 0: %d\n", (runtime_config.socd_pair_0_flags_bits >> bits_pressed_last) & 1);
                    break;

                default:
                    break;
            }
            scan_counter = 0;
        }
    }
}