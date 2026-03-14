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
// Ставит бит номер "bit" в "matrix" равным значению "set_to"
static inline uint8_t set_bit_to(uint8_t matrix, uint8_t bit, bool set_to) {
    if (set_to) {
        matrix |= (1 << bit);
        return matrix;
    }
    matrix &= ~(1 << bit);
    return matrix;
}

// Вспомогательная функция для записи SOCD пар
static inline void socd_mapper_helper(socd_pair_t *pair, uint8_t col, uint8_t row) {
    if (!socd_counter) {
        pair->button_0_pos[0] = col;
        pair->button_0_pos[1] = row;
        socd_counter          = 1;
        return;
    }

    if (socd_counter && (pair->button_0_pos[0] != col || pair->button_0_pos[1] != row)) {
        pair->button_1_pos[0]                    = col;
        pair->button_1_pos[1]                    = row;
        socd_counter                             = 0;
        runtime_config.kb_current_operation_mode = 0;
        return;
    }
}

// Функция для записи SOCD пар. Записывает следующие две нажатые кнопки как SOCD пару в выбранный слот.
void socd_mapper(uint8_t col, uint8_t row) {
    switch (runtime_config.socd_pair_current) {
        case 0: // Слот 0
            socd_mapper_helper(&runtime_config.socd_pair_0, col, row);
            break;

        case 1: // Слот 1
            socd_mapper_helper(&runtime_config.socd_pair_1, col, row);
            break;

        case 2: // Слот 2
            socd_mapper_helper(&runtime_config.socd_pair_2, col, row);
            break;

        default:
            break;
    }
}

/*
bits_key_0_current_state  = 0,
bits_key_1_current_state  = 1,
bits_pressed_first        = 2,
bits_key_0_previous_state = 3,
bits_key_1_previous_state = 4,
bits_marker               = 5
*/

// Если клавиша по адресу входит в SOCD пару, обновляет ее биты raw состояния в runtime_config.socd_pair_X_flags_bits
inline uint8_t socd_update_pair_raw(matrix_row_t current_matrix[], uint8_t col, uint8_t row, uint8_t socd_pairs_flags_bits, socd_pair_t *pair) {
    if (!is_socd_on()) {
        return socd_pairs_flags_bits;
    }

    if (!is_socd_pair_on(pair)) {
        return socd_pairs_flags_bits;
    }

    if (col == pair->button_0_pos[0] && row == pair->button_0_pos[1]) {
        if ((current_matrix[row] >> col) & 1) {
            socd_pairs_flags_bits |= (1 << bits_key_0_current_state);
            return socd_pairs_flags_bits;
        }
        socd_pairs_flags_bits &= ~(1 << bits_key_0_current_state);
        return socd_pairs_flags_bits;
    }

    if (col == pair->button_1_pos[0] && row == pair->button_1_pos[1]) {
        if ((current_matrix[row] >> col) & 1) {
            socd_pairs_flags_bits |= (1 << bits_key_1_current_state);
            return socd_pairs_flags_bits;
        }
        socd_pairs_flags_bits &= ~(1 << bits_key_1_current_state);
        return socd_pairs_flags_bits;
    }
    return socd_pairs_flags_bits;
}

uint8_t socd_perform_pair(matrix_row_t current_matrix[], socd_pair_t *pair, uint8_t socd_pairs_flags_bits) {
    /*
    LAST WINS:
    Если нажата только одна клавиша — выводится она

    Если обе противоположные клавиши удерживаются одновременно,
    активным считается направление последней нажатой клавиши.

    Пока обе клавиши продолжают удерживаться, выход не меняется.

    Если одна из двух клавиш отпущена, активным становится направление
    той клавиши, которая осталась удерживаться.

    Держим в уме, что один тап по клавиши это 3-5 полных сканирований матрицы
    */
    if (!is_socd_on()) {
        return socd_pairs_flags_bits;
    }

    if (!is_socd_pair_on(pair)) {
        return socd_pairs_flags_bits;
    }

    return socd_pairs_flags_bits;
}
