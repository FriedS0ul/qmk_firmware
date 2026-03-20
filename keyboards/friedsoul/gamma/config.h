
#pragma once

// МАТРИЦА
#define MATRIX_COLS 14 // Количество колонок матрицы
#define MATRIX_ROWS 5 // Количество рядов матрицы

#define UNUSED_POSITIONS {{3, 4}, {4, 4}, {7, 4}, {8, 4}} // Тут адреса датчиков в матрице, которые физически не присутствуют на плате 

// МУЛЬТИПЛЕКСОРЫ
#define AMUX_COUNT 2
#define AMUX_MAX_CAPACITY 8
#define AMUX_CURRENT_CAPACITY {8, 6}

#define AMUX_EN_PINS{B5, B7}
#define AMUX_SEL_PINS{A15, B3, B4}

#define AMUX_0_LOGICAL_CHANNELS {2, 1, 0, 4, 6, 7, 5, 3} // Тут стоят физические каналы мультиплексоров в логическом порядке 
#define AMUX_1_LOGICAL_CHANNELS {5, 4, 2, 1, 0, 3}
#define AMUX_LOGICAL_CHANNELS AMUX_0_LOGICAL_CHANNELS, AMUX_1_LOGICAL_CHANNELS

// ПИНЫ
#define MATRIX_ROW_PINS {A10, A9, A8, B15, A4} // Пины рядов матрицы

#define DISCHARGE_PIN A5 // Пин разрядки COM линии

#define ANALOG_READINGS_INPUT B2 // Аналоговый пин на АЦП контроллера

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
#define EECONFIG_KB_DATA_SIZE 256