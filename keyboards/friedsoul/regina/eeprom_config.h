
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "quantum.h"

/*
bool - 1 байт
unt8_t - 1 байт
uint16_t - 2 байта
*/

// Enums для advanced_features_status_bits
enum advanced_features_bit_ids {

    bits_advanced_features_global = 7,
    bits_socd_status_global       = 6,
    bits_actuation_mode_global    = 5

};

enum socd_pair_flags_bit_ids {

    bits_key_0_current_state  = 0,
    bits_key_1_current_state  = 1,
    bits_pressed_first        = 2,

    bits_marker               = 7 // Маркер для bits_pressed_first, изменяется на 1 при первом его изменении

};

// Структуры для записи адресов SOCD пар
typedef struct {
    uint8_t button_0_pos[2]; // {col, row}
    uint8_t button_1_pos[2]; // {col, row}
    uint8_t pair_mode;       // 0 - OFF, 1 - Last wins, 2 - Neutral, 3 - First wins
} socd_pair_t;

// Структура для записи/чтения данных из eeprom
typedef struct {
    uint8_t  fw_level_number;
    uint8_t  console_log_status;                              // Статус лога в консоль | 0 - Выключено,  1 - Данные порогов матрицы, 2 - Данные сканирования, 3 - размер eeprom_config в байтах и данные runtime_config, 4 - Кастом
    uint16_t ceiling_level_per_key[MATRIX_COLS][MATRIX_ROWS]; // Максимальное значение клавиши (Полностью нажата)
    uint16_t actuation_level_global;                          // Точка активации глобальная | 0 - 1023
    uint16_t release_level_global;                            // Точка деактивации глобальная | 0 - 1023
    uint8_t  advanced_features_status_bits;                   // |         7         |      6      |            5          | 4 | 3 | 2 | 1 | 0 |
                                                              // | Advanced features | SOCD Global | Actuation mode global | 0 | 0 | 0 | 0 | 0 |
    socd_pair_t socd_pair_0;                                  // SOCD пара 0
    socd_pair_t socd_pair_1;                                  // SOCD пара 1
    socd_pair_t socd_pair_2;                                  // SOCD пара 2

} eeprom_config_t;

extern eeprom_config_t eeprom_config;

// Структура для runtime данных, обновляется при помощи runtime_renew
typedef struct {
    uint8_t  kb_current_operation_mode;                         // 0 - Обычная работа, 1 - Калибровка порогов, 2 - Запись для SOCD
    uint8_t  console_log_status;                                // Статус лога в консоль // 0 - Выключено,  1 - Данные порогов матрицы, 2 - Данные сканирования, 3 - размер eeprom_config в байтах и данные runtime_config
    uint16_t actuation_level_global;                            // Точка активации глобальная 0 - 1023
    uint16_t release_level_global;                              // Точка деактивации глобальная 0 - 1023
    uint16_t ceiling_level_per_key[MATRIX_COLS][MATRIX_ROWS];   // Максимальное значение клавиши (Полностью нажата)
    uint16_t floor_level_per_key[MATRIX_COLS][MATRIX_ROWS];     // Минимальное значение клавиши (В покое)
    uint16_t actuation_level_per_key[MATRIX_COLS][MATRIX_ROWS]; // Точка активации
    uint16_t release_level_per_key[MATRIX_COLS][MATRIX_ROWS];   // Точка деактивации
    uint8_t  advanced_features_status_bits;                     // |         7         |      6      |            5          | 4 | 3 | 2 | 1 | 0 |
                                                                // | Advanced features | SOCD Global | Actuation mode global | 0 | 0 | 0 | 0 | 0 |
    socd_pair_t socd_pair_0;                                    // SOCD пара 0
    socd_pair_t socd_pair_1;                                    // SOCD пара 1
    socd_pair_t socd_pair_2;                                    // SOCD пара 2
    uint8_t     socd_pair_current;

    uint8_t socd_pair_0_flags_bits;
    uint8_t socd_pair_1_flags_bits;
    uint8_t socd_pair_2_flags_bits;

    bool key_0_prev;
    bool key_1_prev;
    bool key_pressed_first;
    bool key_0_current;
    bool key_1_current;

// Битовые матрицы статуса калибровки
#if (MATRIX_COLS <= 8)
    uint8_t calibration_status_per_key_bits[MATRIX_ROWS];
#elif (MATRIX_COLS <= 16)
    uint16_t calibration_status_per_key_bits[MATRIX_ROWS];
#elif (MATRIX_COLS <= 32)
    uint32_t calibration_status_per_key_bits[MATRIX_ROWS];
#else
#    error "Проверь MATRIX_COLS"
#endif
} runtime_config_t;

extern runtime_config_t runtime_config;

uint16_t log_matrix[MATRIX_COLS][MATRIX_ROWS];
void     logger(void);
void     runtime_renew(void);
void     eeprom_reset(void);
void     save_to_eeprom(void);
void     socd_mapper(uint8_t col, uint8_t row);
bool     is_socd_on(void);
bool     is_socd_pair_on(socd_pair_t *pair);
uint8_t  socd_update_pair_raw(matrix_row_t current_matrix[], uint8_t col, uint8_t row, uint8_t socd_pairs_flags_bits, socd_pair_t *pair);
uint8_t  socd_perform_pair(matrix_row_t current_matrix[], socd_pair_t *pair, uint8_t socd_pairs_flags_bits);