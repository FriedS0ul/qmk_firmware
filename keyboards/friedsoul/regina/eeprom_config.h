
#pragma once 

#include <stdint.h>
#include <stdbool.h>
#include "config.h"


// Структура для записи/чтения данных из eeprom
typedef struct {
    uint8_t console_log_status; // Включен ли вывод лога сканирования в консоль
    uint16_t ceiling_level_per_key[MATRIX_COLS][MATRIX_ROWS]; // Максимальное значение клавиши (Полностью нажата)
    uint16_t actuation_level_global; // Точка активации
    uint16_t release_level_global; // Точка деактивации

} eeprom_config_t;

extern eeprom_config_t eeprom_config;

typedef struct {
    bool is_valid;
    bool calibration_status; // Включена ли калибровка порогов
    uint8_t console_log_status; // Включен ли вывод лога сканирования в консоль
    uint16_t actuation_level_global; // Точка активации глобальная 0 - 1023
    uint16_t release_level_global; // Точка деактивации глобальная 0 - 1023
    uint16_t ceiling_level_per_key[MATRIX_COLS][MATRIX_ROWS]; // Максимальное значение клавиши (Полностью нажата)
    uint16_t floor_level_per_key[MATRIX_COLS][MATRIX_ROWS]; // Минимальное значение клавиши (В покое)
    uint16_t actuation_level_per_key[MATRIX_COLS][MATRIX_ROWS]; // Точка активации
    uint16_t release_level_per_key[MATRIX_COLS][MATRIX_ROWS]; // Точка деактивации

} runtime_config_t;

extern runtime_config_t runtime_config;

void save_to_eeprom(void);