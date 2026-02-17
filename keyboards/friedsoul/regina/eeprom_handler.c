#include "eeprom_config.h"

void eeconfig_init_kb(void){
    // Функция записи ДЕФОЛТНЫХ значений в eeprom
    custom_eeconfig.console_log_status = DEFAULT_CONSOLE_LOG_STATUS; // Включен ли вывод лога сканирования в консоль
    custom_eeconfig.calibration_status = DEFAULT_CALIBRATION_STATUS; // Включена ли калибровка порогов
    custom_eeconfig.actuation_level_global = DEFAULT_ACTUATION_LEVEL; // Точка активации
    custom_eeconfig.release_level_global = DEFAULT_RELEASE_LEVEL; // Точка деактивации
    for (uint8_t col = 0; col < MATRIX_COLS; col++){
        for (uint8_t row = 0; row < MATRIX_ROWS; row++){
            custom_eeconfig.ceiling_level_per_key[col][row] = DEFAULT_CEILING_LEVEL; // Максимальное значение клавиши (Полностью нажата)
        }
    }

    eeconfig
}