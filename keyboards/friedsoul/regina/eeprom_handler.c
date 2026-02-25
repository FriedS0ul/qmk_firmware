#include "eeprom_config.h"
#include "keyboard.h"
#include "eeconfig.h"

// Линейная интерполяция: Y = Y1 + ((X - X1) * (Y2 - Y1)) / (X2 - X1)
uint16_t interpolate(uint16_t Y_start, uint16_t Y_finish, uint16_t X, uint16_t X_start, uint16_t X_finish) {
    return Y_start + ((X - X_start) * (Y_finish - Y_start)) / (X_finish - X_start);
}

// Обновление EEPROM(и eeprom_config) значениями из runtime_config
void save_to_eeprom(void) {
    eeprom_config.actuation_level_global = runtime_config.actuation_level_global;
    eeprom_config.release_level_global   = runtime_config.release_level_global;
    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            eeprom_config.ceiling_level_per_key[col][row] = runtime_config.ceiling_level_per_key[col][row];
            eeprom_config.socd_status_per_key_bits[row]   = runtime_config.socd_status_per_key_bits[row];
        }
    }
    eeconfig_update_kb_datablock(&eeprom_config, 0, sizeof(eeprom_config));
}

// Обновление runtime_config значениями из EEPROM(eeprom_config)
void runtime_renew(void) {
    runtime_config.kb_current_operation_mode = 0;
    runtime_config.socd_status               = eeprom_config.socd_status;
    runtime_config.console_log_status        = eeprom_config.console_log_status;
    runtime_config.actuation_level_global    = eeprom_config.actuation_level_global;
    runtime_config.release_level_global      = eeprom_config.release_level_global;
    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            runtime_config.calibration_status_per_key_bits[row] = 0;
            runtime_config.socd_status_per_key_bits[row]        = eeprom_config.socd_status_per_key_bits[row];
            runtime_config.ceiling_level_per_key[col][row]      = eeprom_config.ceiling_level_per_key[col][row];
            runtime_config.actuation_level_per_key[col][row]    = interpolate(runtime_config.floor_level_per_key[col][row], runtime_config.ceiling_level_per_key[col][row], runtime_config.actuation_level_global, 0, 1023);
            runtime_config.release_level_per_key[col][row]      = interpolate(runtime_config.floor_level_per_key[col][row], runtime_config.ceiling_level_per_key[col][row], runtime_config.release_level_global, 0, 1023);
        }
    }
}

// Перезапись EEPROM дефолтными значениями
void eeprom_reset(void) {
    eeprom_config.socd_status            = 0;
    eeprom_config.console_log_status     = DEFAULT_CONSOLE_LOG_STATUS;
    eeprom_config.fw_level_number        = FIRMWARE_LEVEL_NUMBER;
    eeprom_config.actuation_level_global = DEFAULT_ACTUATION_LEVEL;
    eeprom_config.release_level_global   = DEFAULT_RELEASE_LEVEL;
    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            eeprom_config.ceiling_level_per_key[col][row] = DEFAULT_CEILING_LEVEL;
            eeprom_config.socd_status_per_key_bits[row]   = 0;
        }
    }
    eeconfig_update_kb_datablock(&eeprom_config, 0, sizeof(eeprom_config));
}

// Функция записи дефолтных значений в eeprom при первом запуске или после полного сброса контроллера (НУЖНО СДЕЛАТЬ ПЕРЕЗАПИСЬ ДЕФОЛТАМИ ПРИ НЕСОВПАДЕНИИ ВЕРСИИ ПРОШИВКИ)
void eeconfig_init_kb(void) {
    eeprom_reset();

    eeconfig_init_user();
}

// Функция, которая во время инициализации клавиатуры читает значения из eeprom и заполняет runtime_config на их основе
void keyboard_post_init_kb(void) {
    eeconfig_read_kb_datablock(&eeprom_config, 0, sizeof(eeprom_config));

    runtime_renew();

    keyboard_post_init_user();
}
