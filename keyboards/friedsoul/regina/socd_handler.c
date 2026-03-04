#include "quantum.h"
#include "eeprom_config.h"

static bool socd_counter = 0;
/*
static void socd_mapper_helper(struct slot_t *slot, uint8_t col, uint8_t row) {

    if (!socd_counter) {
        slot->button_0_pos[0] = col;
        slot->button_0_pos[1] = row;
        socd_counter                               = 1;

        return;
    }

    if (socd_counter && (slot->button_0_pos[0] != col || slot->button_0_pos[1] != row)) {
        slot->button_1_pos[0] = col;
        slot->button_1_pos[1] = row;
        socd_counter                               = 0;
        runtime_config.kb_current_operation_mode   = 0;

        return;
    }
}
*/

// Функция для записи SOCD пар. Записывает следующие две нажатые кнопки как SOCD пару в выбранные слот. (ПОТОМ ВЫНЕСТИ В ОТДЕЛЬНЫЙ SOCD_handler.c)
void socd_mapper(uint8_t col, uint8_t row) {
    switch (runtime_config.socd_pair_current) {
        case 0:
            // Слот 0
            if (!socd_counter) {
                runtime_config.socd_pair_0.button_0_pos[0] = col;
                runtime_config.socd_pair_0.button_0_pos[1] = row;
                socd_counter                               = 1;

                break;
            }

            if (socd_counter && (runtime_config.socd_pair_0.button_0_pos[0] != col || runtime_config.socd_pair_0.button_0_pos[1] != row)) {
                runtime_config.socd_pair_0.button_1_pos[0] = col;
                runtime_config.socd_pair_0.button_1_pos[1] = row;
                socd_counter                               = 0;
                runtime_config.kb_current_operation_mode   = 0;

                break;
            }

            break;

        case 1:
            // Слот 1
            if (!socd_counter) {
                runtime_config.socd_pair_1.button_0_pos[0] = col;
                runtime_config.socd_pair_1.button_0_pos[1] = row;
                socd_counter                               = 1;

                break;
            }

            if (socd_counter && (runtime_config.socd_pair_1.button_0_pos[0] != col || runtime_config.socd_pair_1.button_0_pos[1] != row)) {
                runtime_config.socd_pair_1.button_1_pos[0] = col;
                runtime_config.socd_pair_1.button_1_pos[1] = row;
                socd_counter                               = 0;
                runtime_config.kb_current_operation_mode   = 0;

                break;
            }

            break;

        case 2:
            // Слот 2
            if (!socd_counter) {
                runtime_config.socd_pair_2.button_0_pos[0] = col;
                runtime_config.socd_pair_2.button_0_pos[1] = row;
                socd_counter                               = 1;

                break;
            }

            if (socd_counter && (runtime_config.socd_pair_2.button_0_pos[0] != col || runtime_config.socd_pair_2.button_0_pos[1] != row)) {
                runtime_config.socd_pair_2.button_1_pos[0] = col;
                runtime_config.socd_pair_2.button_1_pos[1] = row;
                socd_counter                               = 0;
                runtime_config.kb_current_operation_mode   = 0;

                break;
            }

            break;

        default:
            break;
    }
}

// Меняет current_matrix на основе полученных данных и режима SOCD пары
static inline void socd_handler_helper(matrix_row_t current_matrix[], uint8_t col, uint8_t row, uint8_t opposed_key[2], uint8_t pair_mode) {
    switch (pair_mode) {
        // LAST WINS
        case 0:
            current_matrix[row] |= (1 << col);                        // Нажимаем
            current_matrix[opposed_key[1]] &= ~(1 << opposed_key[0]); // Отпускаем SOCD пару

            break;
        // NEUTRAL
        case 1:
            if ((current_matrix[opposed_key[1]] >> opposed_key[0]) & 1) {
                current_matrix[row] &= ~(1 << col); // Отпускаем текущую
                current_matrix[opposed_key[1]] &= ~(1 << opposed_key[0]); // Отпускаем SOCD пару
                break;
            }
            current_matrix[row] |= (1 << col);                        // Нажимаем
            break;
        // FIRST WINS
        case 2:
            if ((current_matrix[opposed_key[1]] >> opposed_key[0]) & 1) {
                // Не регистрируем нажатие 
                break;
            }
            current_matrix[row] |= (1 << col);                        // Нажимаем
            break;
    }
}

// Проверяет входит ли переданный адрес в SOCD пару и в случае совпадение передает данные в socd_handler_helper
void inline socd_handler(matrix_row_t current_matrix[], uint8_t col, uint8_t row) {
    // Слот 0
    if ((runtime_config.advanced_features_status_bits >> bits_socd_pair_0_status) & 1) {
        if (col == runtime_config.socd_pair_0.button_0_pos[0] && row == runtime_config.socd_pair_0.button_0_pos[1]) {
            socd_handler_helper(current_matrix, col, row, runtime_config.socd_pair_0.button_1_pos, runtime_config.socd_pair_0.pair_mode);
            return;
        }

        if (col == runtime_config.socd_pair_0.button_1_pos[0] && row == runtime_config.socd_pair_0.button_1_pos[1]) {
            socd_handler_helper(current_matrix, col, row, runtime_config.socd_pair_0.button_0_pos, runtime_config.socd_pair_0.pair_mode);
            return;
        }
    }
    // Слот 1
    if ((runtime_config.advanced_features_status_bits >> bits_socd_pair_1_status) & 1) {
        if (col == runtime_config.socd_pair_1.button_0_pos[0] && row == runtime_config.socd_pair_1.button_0_pos[1]) {
            socd_handler_helper(current_matrix, col, row, runtime_config.socd_pair_1.button_1_pos, runtime_config.socd_pair_1.pair_mode);
            return;
        }

        if (col == runtime_config.socd_pair_1.button_1_pos[0] && row == runtime_config.socd_pair_1.button_1_pos[1]) {
            socd_handler_helper(current_matrix, col, row, runtime_config.socd_pair_1.button_0_pos, runtime_config.socd_pair_1.pair_mode);
            return;
        }
    }
    // Слот 2
    if ((runtime_config.advanced_features_status_bits >> bits_socd_pair_2_status) & 1) {
        if (col == runtime_config.socd_pair_2.button_0_pos[0] && row == runtime_config.socd_pair_2.button_0_pos[1]) {
            socd_handler_helper(current_matrix, col, row, runtime_config.socd_pair_2.button_1_pos, runtime_config.socd_pair_2.pair_mode);
            return;
        }

        if (col == runtime_config.socd_pair_2.button_1_pos[0] && row == runtime_config.socd_pair_2.button_1_pos[1]) {
            socd_handler_helper(current_matrix, col, row, runtime_config.socd_pair_2.button_0_pos, runtime_config.socd_pair_2.pair_mode);
            return;
        }
    }
    current_matrix[row] |= (1 << col); // Нажимаем (Если слоты SOCD неактивны)
}