
#pragma once 

#include <stdint.h>
#include <stdbool.h>
#include "config.h"

// Структура для записи/чтения данных из eeprom
typedef struct{
    bool console_log_status; // Включен ли вывод лога сканирования в консоль
    bool calibration_status; // Включена ли калибровка порогов
    uint16_t ceiling_level_per_key[MATRIX_COLS][MATRIX_ROWS]; // Максимальное значение клавиши (Полностью нажата)
    uint16_t actuation_level_global; // Точка активации
    uint16_t release_level_global; // Точка деактивации

} custom_eeconfig_struct_t;

extern custom_eeconfig_struct_t custom_eeconfig;