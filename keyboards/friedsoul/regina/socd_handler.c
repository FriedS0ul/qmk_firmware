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
            socd_pairs_flags_bits |= (1 << bits_key_0_after_socd);
            return socd_pairs_flags_bits;
        }
        socd_pairs_flags_bits &= ~(1 << bits_key_0_after_socd);
        return socd_pairs_flags_bits;
    }

    if (col == pair->button_1_pos[0] && row == pair->button_1_pos[1]) {
        if ((current_matrix[row] >> col) & 1) {
            socd_pairs_flags_bits |= (1 << bits_key_1_after_socd);
            return socd_pairs_flags_bits;
        }
        socd_pairs_flags_bits &= ~(1 << bits_key_1_after_socd);
        return socd_pairs_flags_bits;
    }
    return socd_pairs_flags_bits;
}

/*
    bits_key_0_after_socd  = 0,
    bits_key_1_after_socd  = 1,
    bits_pressed_last      = 2,
    bits_key_0_before_socd = 3,
    bits_key_1_before_socd = 4,

    bits_marker = 7
*/

void socd_perform_pair(matrix_row_t current_matrix[], socd_pair_t *pair, uint8_t socd_pairs_flags_bits) {
    /*
    LAST WINS:
    Если нажата только одна клавиша — выводится она

    Если обе противоположные клавиши удерживаются одновременно,
    активным считается направление последней нажатой клавиши.

    Пока обе клавиши продолжают удерживаться, выход не меняется.

    Если одна из двух клавиш отпущена, активным становится направление
    той клавиши, которая осталась удерживаться.

    Держим в уме, что один тап по клавише это 3-5 полных сканирований матрицы
    */
    if (!is_socd_on()) {
        return;
    }

    if (!is_socd_pair_on(pair)) {
        return;
    }

    bool key_0 = (current_matrix[pair->button_0_pos[1]] >> pair->button_0_pos[0]) & 1;
    bool key_1 = (current_matrix[pair->button_1_pos[1]] >> pair->button_1_pos[0]) & 1;

    bool key_0_is_pressed = key_0 && !((socd_pairs_flags_bits >> bits_key_0_before_socd) & 1);
    bool key_1_is_pressed = key_1 && !((socd_pairs_flags_bits >> bits_key_1_before_socd) & 1);

    bool key_0_is_released = !key_0 && ((socd_pairs_flags_bits >> bits_key_0_before_socd) & 1);
    bool key_1_is_released = !key_1 && ((socd_pairs_flags_bits >> bits_key_1_before_socd) & 1);

    bool key_0_is_held = key_0 && ((socd_pairs_flags_bits >> bits_key_0_before_socd) & 1);
    bool key_1_is_held = key_1 && ((socd_pairs_flags_bits >> bits_key_1_before_socd) & 1);

    bool pressed_last = 0;

    if (key_0_is_pressed && key_1_is_pressed) {
        uprintf("\r\n");
        uprintf("Both keys pressed at the same time, default one is %d", DEFAULT_SOCD_KEY);
        uprintf("\r\n");
        pressed_last = DEFAULT_SOCD_KEY;
    }

    if (key_0_is_pressed) {
        pressed_last = 0;
    }

    if (key_1_is_pressed) {
        pressed_last = 1;
    }

    if (key_0_is_released && key_1_is_held) {
        key_1_is_pressed = 1;
    }

    if (key_1_is_released && key_0_is_held) {
        key_0_is_pressed = 1;
    }

    set_bit_to(socd_pairs_flags_bits, bits_key_0_before_socd, key_0); // Ставим before_socd = текущее состояние
    set_bit_to(socd_pairs_flags_bits, bits_key_1_before_socd, key_1); // Ставим before_socd = текущее состояние
    set_bit_to(socd_pairs_flags_bits, bits_pressed_last, pressed_last);

    if (!(key_0 && key_1)) {
        return;
    }

    // pressed_last == 0
    if (!pressed_last) {
        switch (pair->pair_mode) {
            case 1: // Last wins

                current_matrix[pair->button_1_pos[1]] &= ~(1 << pair->button_1_pos[0]);
                break;

            case 2: // Neutral

                current_matrix[pair->button_0_pos[1]] &= ~(1 << pair->button_0_pos[0]);
                current_matrix[pair->button_1_pos[1]] &= ~(1 << pair->button_1_pos[0]);
                break;

            case 3: // First wins

                current_matrix[pair->button_0_pos[1]] &= ~(1 << pair->button_0_pos[0]);
                break;
        }
        return;
    }

    // pressed_last == 1
    switch (pair->pair_mode) {
        case 1: // Last wins

            current_matrix[pair->button_0_pos[1]] &= ~(1 << pair->button_0_pos[0]);
            break;

        case 2: // Neutral

            current_matrix[pair->button_0_pos[1]] &= ~(1 << pair->button_0_pos[0]);
            current_matrix[pair->button_1_pos[1]] &= ~(1 << pair->button_1_pos[0]);
            break;

        case 3: // First wins

            current_matrix[pair->button_1_pos[1]] &= ~(1 << pair->button_1_pos[0]);
            break;
    }
    return;
}
