
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "config.h"

/*
bool - 1 байт
unt8_t - 1 байт
uint16_t - 2 байта
*/

// Структура для записи/чтения данных из eeprom
typedef struct {
    uint8_t  fw_level_number;                                 // Версия прошивки
    uint8_t  console_log_status;                              // Статус лога в консоль // 0 - Выключено,  1 - Данные порогов матрицы, 2 - Данные сканирования, 3 - размер eeprom_config в байтах и данные runtime_config
    uint16_t ceiling_level_per_key[MATRIX_COLS][MATRIX_ROWS]; // Максимальное значение клавиши (Полностью нажата)
    uint16_t actuation_level_global;                          // Точка активации глобальная 0 - 1023
    uint16_t release_level_global;                            // Точка деактивации глобальная 0 - 1023
    uint8_t  socd_keys_addresses[2][2];                       // {{col, row}, {col, row}}

    // ЛИБО СДЕЛАТЬ ВЛОЖЕННУЮ СТРУКТУРУ С ДВУМЯ МАССИВАМИ ДЛЯ col и row
} eeprom_config_t;

extern eeprom_config_t eeprom_config;

// Структура для runtime данных, обновляется при сохранении в eeprom
typedef struct {
    uint8_t  kb_current_operation_mode;                         // 0 - Обычная работа, 1 - Калибровка порогов, 2 - Запись для SOCD
    uint8_t  console_log_status;                                // Статус лога в консоль // 0 - Выключено,  1 - Данные порогов матрицы, 2 - Данные сканирования, 3 - размер eeprom_config в байтах и данные runtime_config
    uint16_t actuation_level_global;                            // Точка активации глобальная 0 - 1023
    uint16_t release_level_global;                              // Точка деактивации глобальная 0 - 1023
    uint16_t ceiling_level_per_key[MATRIX_COLS][MATRIX_ROWS];   // Максимальное значение клавиши (Полностью нажата)
    uint16_t floor_level_per_key[MATRIX_COLS][MATRIX_ROWS];     // Минимальное значение клавиши (В покое)
    uint16_t actuation_level_per_key[MATRIX_COLS][MATRIX_ROWS]; // Точка активации
    uint16_t release_level_per_key[MATRIX_COLS][MATRIX_ROWS];   // Точка деактивации
    uint8_t  socd_status;                                       // 0 - Выкл, 1 - Вкл // Если сумма элементов socd_keys_addresses != 0
    uint8_t  socd_keys_addresses[2][2];                         // {{col, row}, {col, row}}

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

void runtime_renew(void);
void eeprom_reset(void);
void save_to_eeprom(void);