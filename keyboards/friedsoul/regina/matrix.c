
#include "stdbool.h"
#include "quantum.h"
#include "config.h"
#include "gpio.h"
#include "analog.h"
#include <print.h>
#include "eeprom_config.h"

static const pin_t row_pins[] = MATRIX_ROW_PINS;
static uint16_t    log_matrix[MATRIX_COLS][MATRIX_ROWS];
static uint16_t    scan_counter = 0;

eeprom_config_t  eeprom_config;
runtime_config_t runtime_config;

void adc_int(void) {
    palSetLineMode(ANALOG_READINGS_INPUT, PAL_MODE_INPUT_ANALOG);
}

void pins_init(void) {
    gpio_write_pin_low(DISCHARGE_PIN);
    gpio_set_pin_output(DISCHARGE_PIN);

    for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
        gpio_write_pin_low(row_pins[i]);
        gpio_set_pin_output(row_pins[i]);
    }
}

// Вывод лога кажные N полных сканирований матрицы в консоль
void logger(void) {
    if (runtime_config.console_log_status != 0) {
        if (scan_counter < DEFAULT_CONSOLE_LOG_FREQUENCY) {
            scan_counter++;
        } else {
            switch (runtime_config.console_log_status) {
                case 1:
                    uprintf("\r\n");
                    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                            uprintf("%d, %d Floor: %d Ceiling: %d Actuation: %d Release: %d", col, row, runtime_config.floor_level_per_key[col][row], runtime_config.ceiling_level_per_key[col][row], runtime_config.actuation_level_per_key[col][row], runtime_config.release_level_per_key[col][row]);
                            uprintf("\r\n");
                        }
                    }
                    break;

                case 2:
                    uprintf("\r\n");
                    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                            uprintf("COL %d ROW %d: %u", col, row, log_matrix[col][row]);
                            uprintf("\r\n");
                        }
                    }
                    break;

                case 3:
                    uprintf("\r\n");
                    uprintf("EEPROM CURRENT SIZE: %zu byte \n", sizeof(eeprom_config));
                    uprintf("\r\n");
                    uprintf("FIRMWARE LEVEL %d\n", eeprom_config.fw_level_number);
                    uprintf("CURRENT OPERATION MODE %d\n", runtime_config.kb_current_operation_mode);
                    uprintf("LOG STATUS %d\n", runtime_config.console_log_status);
                    uprintf("ACTUATION GLOBAL %d\n", runtime_config.actuation_level_global);
                    uprintf("RELEASE GLOBAL %d\n", runtime_config.release_level_global);
                    uprintf("\r\n");
                    break;

                default:
                    break;
            }
            scan_counter = 0;
        }
    }
}

// Зарядка ряда для сканирования
void ec_sw_charge(uint8_t row) {
    gpio_write_pin_high(row_pins[row]);

    gpio_set_pin_input(DISCHARGE_PIN);
}

// Разрядка COM линии после сканирования
void ec_sw_discharge(uint8_t row) {
    gpio_write_pin_low(row_pins[row]);

    gpio_write_pin_low(DISCHARGE_PIN);
    gpio_set_pin_output(DISCHARGE_PIN);

    wait_us(DISCHARGE_TIME_US);
}

// Сканирование конкретного датчика по адресу в матрице
uint16_t ec_sw_scan(uint8_t col, uint8_t row) {
    uint16_t raw_adc_readings = 0;

    gpio_write_pin_low(row_pins[row]);

    gpio_set_pin_input(DISCHARGE_PIN);

    gpio_write_pin_high(row_pins[row]);

    raw_adc_readings = analogReadPin(ANALOG_READINGS_INPUT); // Возможно стоит сделать атомарный блок для сканирования + еще поработать над логикой для минимизации шума

    gpio_write_pin_low(DISCHARGE_PIN);
    gpio_set_pin_output(DISCHARGE_PIN);

    wait_us(DISCHARGE_TIME_US);

    log_matrix[col][row] = raw_adc_readings; // Логирования сканирований

    return raw_adc_readings;
}

// Семплинг нижнего порога клавиши (В покое)
void ec_floor_sample(void) {
    uint16_t raw_adc_readings = 0;

    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            runtime_config.floor_level_per_key[col][row] = 0;
        }
    }

    for (uint8_t count = 0; count < BOTTOM_LEVEL_SAMPLING_COUNT; count++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                raw_adc_readings = ec_sw_scan(col, row);
                if (raw_adc_readings > runtime_config.floor_level_per_key[col][row]) {
                    runtime_config.floor_level_per_key[col][row] = raw_adc_readings;
                }
            }
        }
    }
}

// Функция сканирования и обновления current matrix
bool ec_matrix_scan(matrix_row_t current_matrix[]) {
    bool has_changed = false;

    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            switch (runtime_config.kb_current_operation_mode) {
                // Нормальная работа
                case 0: {
                    uint16_t raw_adc_readings  = ec_sw_scan(col, row);             // Получаем данные сканирования конкретного датчика
                    uint8_t  key_current_state = (current_matrix[row] >> col) & 1; // Запрашиваем текущее состояние клавиши (нажата или отпущена)

                    if (raw_adc_readings <= runtime_config.floor_level_per_key[col][row]) {
                        break;
                    }

                    if (raw_adc_readings > runtime_config.actuation_level_per_key[col][row] && key_current_state == 0) {
                        current_matrix[row] |= (1 << col); // Нажимаем
                        has_changed = true;
                    }

                    if (raw_adc_readings < runtime_config.release_level_per_key[col][row] && key_current_state == 1) {
                        current_matrix[row] &= ~(1 << col); // Отпускаем
                        has_changed = true;
                    }

                    break;
                }
                // Калибровка порогов
                case 1: {
                    uint8_t  key_calibration_status = (runtime_config.calibration_status_per_key_bits[row] >> col) & 1; // Запрашиваем статус калибровки конкретной клавиши
                    uint16_t raw_adc_readings       = ec_sw_scan(col, row);                                             // Получаем данные сканирования конкретного датчика

                    if (key_calibration_status == 0) {
                        runtime_config.ceiling_level_per_key[col][row] = 0;
                        runtime_config.ceiling_level_per_key[col][row] = raw_adc_readings;
                        runtime_config.calibration_status_per_key_bits[row] |= (1 << col);
                    }

                    if (runtime_config.ceiling_level_per_key[col][row] < raw_adc_readings) {
                        runtime_config.ceiling_level_per_key[col][row] = raw_adc_readings;
                    }

                    break;
                }
                // Запись SOCD
                case 2: {
                    // uint8_t key_socd_status = (runtime_config.socd_status_per_key_bits[row] >> col) & 1; // Запрашиваем статус SOCD конкретной клавиши

                    break;
                }
                // Ошибка
                default:

                    break;
            }
        }
    }
    return has_changed;
}

// Инициализация матрицы СТАНДАРТНАЯ
void matrix_init_custom(void) {
    adc_int();

    pins_init();

    ec_floor_sample();
}

// Сканирование матрицы СТАНДАРТНАЯ
bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    bool matrix_has_changed = ec_matrix_scan(current_matrix);

    logger();
    return matrix_has_changed;
}