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

void socd_perform_new(matrix_row_t current_matrix[], uint8_t socd_keys_raw_states_bits) {
    bool event_key_0_pressed;
    bool event_key_1_pressed;

    if (runtime_config.socd_pair_0.pair_mode != 0) {
        event_key_0_pressed = (((runtime_config.socd_pair_0_flags_bits >> bits_key_0_previous_state) & 1) == 0 && ((socd_keys_raw_states_bits >> bits_slot_0_key_0) & 1) == 1);
        event_key_1_pressed = (((runtime_config.socd_pair_0_flags_bits >> bits_key_1_previous_state) & 1) == 0 && ((socd_keys_raw_states_bits >> bits_slot_0_key_1) & 1) == 1);

        if (event_key_0_pressed && ((socd_keys_raw_states_bits >> bits_slot_0_key_1) & 1) == 0) {
            runtime_config.socd_pair_0_flags_bits &= ~(1 << bits_winner_key);
        }

        if (event_key_1_pressed && ((socd_keys_raw_states_bits >> bits_slot_0_key_0) & 1) == 0) {
            runtime_config.socd_pair_0_flags_bits |= (1 << bits_winner_key);
        }
    }
}

void socd_perform(matrix_row_t current_matrix[], uint8_t socd_key_0_now, uint8_t socd_key_1_now) {
    uint8_t event_pressed_key_0 = (runtime_config.pair_0_key_0_previous_state == 0 && socd_key_0_now == 1);
    uint8_t event_pressed_key_1 = (runtime_config.pair_0_key_1_previous_state == 0 && socd_key_1_now == 1);

    if (event_pressed_key_0 == 1 && socd_key_1_now == 0) {
        runtime_config.pair_0_key_winnner = 0;
    }

    if (event_pressed_key_1 == 1 && socd_key_0_now == 0) {
        runtime_config.pair_0_key_winnner = 1;
    }

    if (socd_key_0_now == 1 && socd_key_1_now == 1) {
        switch (runtime_config.pair_0_key_winnner) {
            case 0:
                // тут меняем матрицу в соответствии с режимом
                break;
            case 1:
                // тут меняем матрицу в соответствии с режимом
                break;
                ...

                    default : break;
        }
    }

    runtime_config.pair_0_key_0_previous_state = socd_key_0_now;
    runtime_config.pair_0_key_1_previous_state = socd_key_1_now;
}
