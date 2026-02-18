#include "eeprom_config.h"
#include "eeconfig.h"

void interpolate(void){
    
}

// Функция записи дефолтных значений в eeprom при первом запуске или после полного сброса контроллера
void eeconfig_init_kb(void){
    custom_eeconfig.console_log_status = DEFAULT_CONSOLE_LOG_STATUS; // Включен ли вывод лога сканирования в консоль
    custom_eeconfig.calibration_status = DEFAULT_CALIBRATION_STATUS; // Включена ли калибровка порогов
    custom_eeconfig.actuation_level_global = DEFAULT_ACTUATION_LEVEL; // Точка активации
    custom_eeconfig.release_level_global = DEFAULT_RELEASE_LEVEL; // Точка деактивации
    for (uint8_t col = 0; col < MATRIX_COLS; col++){
        for (uint8_t row = 0; row < MATRIX_ROWS; row++){
            custom_eeconfig.ceiling_level_per_key[col][row] = DEFAULT_CEILING_LEVEL; // Максимальное значение клавиши (Полностью нажата)
        }
    }
    eeconfig_update_kb_datablock(&custom_eeconfig, 0, sizeof(custom_eeconfig)); // Записываем

    eeconfig_init_user(); // Эта хуйня, вроде, заполняет нулями user часть eeprom
}

void keyboard_post_init_kb(void){
    if (eeconfig_is_kb_datablock_valid()){
       eeconfig_read_kb_datablock(&custom_eeconfig, 0, sizeof(custom_eeconfig)); // Читаем значения из eeprom
    }
    else {
        eeconfig_init();
        eeconfig_init_kb();
    }

    keyboard_post_init_user();
}
