#include "eeprom_config.h"
#include "keyboard.h"
#include "eeconfig.h"

// Линейная интерполяция: Y = Y1 + ((X - X1) * (Y2 - Y1)) / (X2 - X1)
uint16_t interpolate(uint16_t Y_start, uint16_t Y_finish, uint16_t X, uint16_t X_start, uint16_t X_finish) {
    return Y_start + ((X - X_start) * (Y_finish - Y_start)) / (X_finish - X_start);
}

// Запись запись данных из runtime_config в eeprom_config и сохранение в eeprom
void save_to_eeprom(void) {
    eeprom_config.console_log_status = runtime_config.console_log_status;
    eeprom_config.actuation_level_global = runtime_config.actuation_level_global;
    eeprom_config.release_level_global = runtime_config.release_level_global;
    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            eeprom_config.ceiling_level_per_key[col][row] = runtime_config.ceiling_level_per_key[col][row];
            // Пересчитываем actuation и release при сохранении, сбрасываем флаги калибровки
            runtime_config.calibration_status = DEFAULT_CALIBRATION_STATUS;
            runtime_config.calibration_status_per_key[col][row] = true;
            runtime_config.actuation_level_per_key[col][row] = interpolate(runtime_config.floor_level_per_key[col][row], runtime_config.ceiling_level_per_key[col][row], runtime_config.actuation_level_global, 0, 1023);
            runtime_config.release_level_per_key[col][row]   = interpolate(runtime_config.floor_level_per_key[col][row], runtime_config.ceiling_level_per_key[col][row], runtime_config.release_level_global, 0, 1023);
        }
    }

    eeconfig_update_kb_datablock(&eeprom_config, 0, sizeof(eeprom_config));
}

// Чтение данных из eeprom и обновление runtime_config
void read_from_eeprom(void) {
    eeconfig_read_kb_datablock(&eeprom_config, 0, sizeof(eeprom_config));

    runtime_config.console_log_status = eeprom_config.console_log_status;
    runtime_config.actuation_level_global = eeprom_config.actuation_level_global;
    runtime_config.release_level_global = eeprom_config.release_level_global;
    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            runtime_config.ceiling_level_per_key[col][row] = eeprom_config.ceiling_level_per_key[col][row];
            runtime_config.actuation_level_per_key[col][row] = interpolate(runtime_config.floor_level_per_key[col][row], runtime_config.ceiling_level_per_key[col][row], runtime_config.actuation_level_global, 0, 1023);
            runtime_config.release_level_per_key[col][row]   = interpolate(runtime_config.floor_level_per_key[col][row], runtime_config.ceiling_level_per_key[col][row], runtime_config.release_level_global, 0, 1023);
        }
    }
}

// Функция записи дефолтных значений в eeprom при первом запуске или после полного сброса контроллера (НУЖНО СДЕЛАТЬ ПЕРЕЗАПИСЬ ДЕФОЛТАМИ ПРИ НЕСОВПАДЕНИИ ВЕРСИИ ПРОШИВКИ)
void eeconfig_init_kb(void) {
    eeprom_config.console_log_status     = DEFAULT_CONSOLE_LOG_STATUS; // Включен ли вывод лога сканирования в консоль
    eeprom_config.actuation_level_global = DEFAULT_ACTUATION_LEVEL;    // Точка активации
    eeprom_config.release_level_global   = DEFAULT_RELEASE_LEVEL;      // Точка деактивации
    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            eeprom_config.ceiling_level_per_key[col][row] = DEFAULT_CEILING_LEVEL; // Максимальное значение клавиши (Полностью нажата)
        }
    }
    eeconfig_update_kb_datablock(&eeprom_config, 0, sizeof(eeprom_config));

    eeconfig_init_user(); // Эта хуйня заполняет нулями user часть DWORD eeprom. Нужна тк в исходниках eeconfig_init_kb вызывает ее
}

// Функция, которая во время инициализации клавиатуры читает значения из eeprom и заполняет runtime_config на их основе
void keyboard_post_init_kb(void) {

    eeconfig_read_kb_datablock(&eeprom_config, 0, sizeof(eeprom_config));

    runtime_config.is_valid           = eeconfig_is_kb_datablock_valid();
    runtime_config.calibration_status = DEFAULT_CALIBRATION_STATUS;
    runtime_config.console_log_status = eeprom_config.console_log_status;
    runtime_config.actuation_level_global = eeprom_config.actuation_level_global;
    runtime_config.release_level_global = eeprom_config.release_level_global;
    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            runtime_config.calibration_status_per_key[col][row] = true;
            runtime_config.ceiling_level_per_key[col][row] = eeprom_config.ceiling_level_per_key[col][row];
            runtime_config.actuation_level_per_key[col][row] = interpolate(runtime_config.floor_level_per_key[col][row], runtime_config.ceiling_level_per_key[col][row], runtime_config.actuation_level_global, 0, 1023);
            runtime_config.release_level_per_key[col][row]   = interpolate(runtime_config.floor_level_per_key[col][row], runtime_config.ceiling_level_per_key[col][row], runtime_config.release_level_global, 0, 1023);
        }
    }

    keyboard_post_init_user();
}
