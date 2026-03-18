
#pragma once

// МАТРИЦА
#define MATRIX_COLS 1 // Количество колонок матрицы
#define MATRIX_ROWS 5 // Количество рядов матрицы

// ПИНЫ
#define MATRIX_ROW_PINS {A10, A8, A3, A2, A1} // Пины рядов матрицы

#define DISCHARGE_PIN A4 // Пин разрядки COM линии

#define ANALOG_READINGS_INPUT A7 // Аналоговый пин на АЦП контроллера

// ЫЫЫЫ ЦЫФЕРКИ (Не изменяй, если не знаешь, что они делают)
#define DISCHARGE_TIME_US 10

#define FLOOR_LEVEL_SAMPLING_COUNT 50

#define DEFAULT_ACTUATION_LEVEL 100
#define DEFAULT_RELEASE_LEVEL 50
#define DEFAULT_CEILING_LEVEL 1023

#define DEFAULT_CONSOLE_LOG_STATUS 0        // 0 - Выключено,  1 - Данные порогов матрицы, 2 - Данные сканирования, 3 - размер eeprom_config и данные runtime_config, 4 - Данные SOCD пар
#define DEFAULT_CONSOLE_LOG_FREQUENCY 5000 // Раз в сколько полных сканирований матрицы данные будут выводится в консоль. Частота зависит от размера матрица. Рекомендую не ниже 1000

#define DEFAULT_SOCD_KEY 0 // Какая клавиша будет считаться нажатой последней, если в одном цикле сканирования матрицы было зарегистрировано нажатие обеих
#define FIRMWARE_LEVEL_NUMBER 1
#define EECONFIG_KB_DATA_SIZE 64