
#pragma once 

// MATRIX
#define MATRIX_COLS 1
#define MATRIX_ROWS 5

// PINS
#define MATRIX_ROW_PINS {A10, A8, A3, A2, A1}

#define DISCHARGE_PIN A4

#define ANALOG_READINGS_INPUT A7

// NUMBERS
#define DISCHARGE_TIME_US 10

#define BOTTOM_LEVEL_SAMPLING_COUNT 50
#define NOISE_OFFSET 50

#define DEFAULT_ACTUATION_LEVEL 100
#define DEFAULT_RELEASE_LEVEL 50
#define DEFAULT_CEILING_LEVEL 1023

#define DEFAULT_CONSOLE_LOG_STATUS 1 // 0 - Выключено,  1 - Данные порогов матрицы, 2 - Данные сканирования, 3 - размер eeprom_config в байтах
#define CONSOLE_LOG_FREQUENCY 5000 // Раз в сколько полных сканирований матрицы данные будут выводится в консоль. Частота зависит от размера матрица. Рекомендую не ниже 1000


#define DEFAULT_CALIBRATION_STATUS false

#define EECONFIG_KB_DATA_SIZE 16