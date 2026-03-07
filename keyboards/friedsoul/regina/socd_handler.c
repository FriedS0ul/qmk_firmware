#include "quantum.h"
#include "eeprom_config.h"

static bool socd_counter = 0;

// Возвращает статус глобального флага активности SOCD
bool is_socd_on(void) {
    if (((runtime_config.advanced_features_status_bits >> bits_advanced_features_global) & 1) && ((runtime_config.advanced_features_status_bits >> bits_socd_status_global) & 1)) {
        return true;
    }
    return false;
}

// Возвращает статус конкретной SOCD пары
bool is_socd_pair_on(socd_pair_t *pair) {
    if (pair->pair_mode != 0) {
        return true;
    }
    return false;
}

static inline uint8_t set_bit_to(uint8_t matrix, uint8_t bit, bool set_to) {
    if (set_to) {
        matrix |= (1 << bit);
        return matrix;
    }
    matrix &= ~(1 << bit);
    return matrix;
}

// Если клавиша по адресу входит в SOCD пару, обновляет ее биты raw состояния в runtime_config.socd_pair_X_flags_bits
uint8_t socd_update_pair_raw(matrix_row_t current_matrix[], uint8_t col, uint8_t row, uint8_t socd_pairs_flags_bits, socd_pair_t *pair) {
    if (!is_socd_on()) {
        return socd_pairs_flags_bits;
    }

    if (!is_socd_pair_on(pair)) {
        return socd_pairs_flags_bits;
    }

    if (col == pair->button_0_pos[0] && row == pair->button_0_pos[1]) {
        if ((current_matrix[row] >> col) & 1) {
            socd_pairs_flags_bits |= (1 << bits_key_0_raw);
            return socd_pairs_flags_bits;
        }
        socd_pairs_flags_bits &= ~(1 << bits_key_0_raw);
        return socd_pairs_flags_bits;
    }

    if (col == pair->button_1_pos[0] && row == pair->button_1_pos[1]) {
        if ((current_matrix[row] >> col) & 1) {
            socd_pairs_flags_bits |= (1 << bits_key_1_raw);
            return socd_pairs_flags_bits;
        }
        socd_pairs_flags_bits &= ~(1 << bits_key_1_raw);
        return socd_pairs_flags_bits;
    }
    return socd_pairs_flags_bits;
}

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

uint8_t socd_perform_pair(matrix_row_t current_matrix[], socd_pair_t *pair, uint8_t socd_pairs_flags_bits) {
    if (!is_socd_on()) {
        return socd_pairs_flags_bits;
    }

    if (!is_socd_pair_on(pair)) {
        return socd_pairs_flags_bits;
    }

    bool event_key_0_pressed = ((socd_pairs_flags_bits >> bits_key_0_previous_state) & 1) == 0 && ((socd_pairs_flags_bits >> bits_key_0_raw) & 1) == 1;
    bool event_key_1_pressed = ((socd_pairs_flags_bits >> bits_key_1_previous_state) & 1) == 0 && ((socd_pairs_flags_bits >> bits_key_1_raw) & 1) == 1;

    // uprintf("Key 0: %d, Key 1: %d\n", event_key_0_pressed, event_key_1_pressed);

    if (event_key_0_pressed == 1 && event_key_1_pressed == 1) {
        socd_pairs_flags_bits &= ~(1 << bits_pressed_first); //
    }

    if (event_key_0_pressed && ((socd_pairs_flags_bits >> bits_key_1_raw) & 1) == 0) {
        socd_pairs_flags_bits &= ~(1 << bits_pressed_first);
    }

    if (event_key_1_pressed && ((socd_pairs_flags_bits >> bits_key_0_raw) & 1) == 0) {
        socd_pairs_flags_bits |= (1 << bits_pressed_first);
    }

    if (!(((socd_pairs_flags_bits >> bits_key_0_raw) & 1) == 1 && ((socd_pairs_flags_bits >> bits_key_1_raw) & 1) == 1)) { // Если не зафиксировано нажатия обеих клавиш одновременно
        socd_pairs_flags_bits = set_bit_to(socd_pairs_flags_bits, bits_key_0_previous_state, ((socd_pairs_flags_bits >> bits_key_0_raw) & 1));
        socd_pairs_flags_bits = set_bit_to(socd_pairs_flags_bits, bits_key_1_previous_state, ((socd_pairs_flags_bits >> bits_key_1_raw) & 1));

        return socd_pairs_flags_bits;
    }

    // KEY 0 WON
    if (!((socd_pairs_flags_bits >> bits_pressed_first) & 1)) {
        switch (pair->pair_mode) {
            case 1: // LAST WINS
                uprintf("KEY 0 WON, last wins");
                current_matrix[pair->button_0_pos[1]] &= ~(1 << pair->button_0_pos[0]);
                current_matrix[pair->button_1_pos[1]] |= (1 << pair->button_1_pos[0]);
                break;

            case 2: // NEUTRAL
                uprintf("KEY 0 WON, neutral\n");
                current_matrix[pair->button_0_pos[1]] &= ~(1 << pair->button_0_pos[0]);
                current_matrix[pair->button_1_pos[1]] &= ~(1 << pair->button_1_pos[0]);
                break;

            case 3: // FIRST WINS
                uprintf("KEY 0 WON, first wins");
                current_matrix[pair->button_0_pos[1]] |= (1 << pair->button_0_pos[0]);
                current_matrix[pair->button_1_pos[1]] &= ~(1 << pair->button_1_pos[0]);
                break;
        }

        socd_pairs_flags_bits = set_bit_to(socd_pairs_flags_bits, bits_key_0_previous_state, ((socd_pairs_flags_bits >> bits_key_0_raw) & 1));
        socd_pairs_flags_bits = set_bit_to(socd_pairs_flags_bits, bits_key_1_previous_state, ((socd_pairs_flags_bits >> bits_key_1_raw) & 1));

        return socd_pairs_flags_bits;
    }

    // KEY 1 WON
    switch (pair->pair_mode) {
        case 1: // LAST WINS
            uprintf("KEY 1 WON, last wins");
            current_matrix[pair->button_0_pos[1]] |= (1 << pair->button_0_pos[0]);
            current_matrix[pair->button_1_pos[1]] &= ~(1 << pair->button_1_pos[0]);
            break;

        case 2: // NEUTRAL
            uprintf("KEY 1 WON, neutral\n");
            current_matrix[pair->button_0_pos[1]] &= ~(1 << pair->button_0_pos[0]);
            current_matrix[pair->button_1_pos[1]] &= ~(1 << pair->button_1_pos[0]);
            break;

        case 3: // FIRST WINS
            uprintf("KEY 1 WON, first wins");
            current_matrix[pair->button_0_pos[1]] &= ~(1 << pair->button_0_pos[0]);
            current_matrix[pair->button_1_pos[1]] |= (1 << pair->button_1_pos[0]);
            break;
    }

    socd_pairs_flags_bits = set_bit_to(socd_pairs_flags_bits, bits_key_0_previous_state, ((socd_pairs_flags_bits >> bits_key_0_raw) & 1));
    socd_pairs_flags_bits = set_bit_to(socd_pairs_flags_bits, bits_key_1_previous_state, ((socd_pairs_flags_bits >> bits_key_1_raw) & 1));

    return socd_pairs_flags_bits;
}
