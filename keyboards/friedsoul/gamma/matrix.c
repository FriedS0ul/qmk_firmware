
#include "stdbool.h"
#include "quantum.h"
#include "config.h"
#include "gpio.h"
#include "analog.h"
#include <print.h>
#include "eeprom_config.h"

static const pin_t   row_pins[]                                        = MATRIX_ROW_PINS;
static const pin_t   mux_en_pins[]                                     = MUX_EN_PINS;
static const pin_t   mux_sel_pins[]                                    = MUX_SEL_PINS;
static const uint8_t mux_logical_channels[MUX_COUNT][MUX_MAX_CAPACITY] = MUX_LOGICAL_CHANNELS;
static const uint8_t mux_current_capacity[MUX_COUNT]                   = MUX_CURRENT_CAPACITY;

#ifdef UNUSED_ADRESSES
static const uint8_t matrix_unused_adresses[][2] = UNUSED_ADRESSES;
static inline bool   matrix_address_unused(uint8_t col_matrix, uint8_t row) {
    for (uint8_t i = 0; i < (sizeof(matrix_unused_adresses) / sizeof(matrix_unused_adresses[0][0])); i++) {
        if (matrix_unused_adresses[i][0] == col_matrix && matrix_unused_adresses[i][1] == row) {
            return true;
        }
    }
    return false;
}
#endif

eeprom_config_t  eeprom_config;
runtime_config_t runtime_config;

static inline void adc_int(void) {
    palSetLineMode(ANALOG_READINGS_INPUT, PAL_MODE_INPUT_ANALOG);
}

static inline void pins_init(void) {
    gpio_write_pin_low(DISCHARGE_PIN);
    gpio_set_pin_output(DISCHARGE_PIN);

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        gpio_write_pin_low(row_pins[row]);
        gpio_set_pin_output(row_pins[row]);
    }
}
// Ставим
static inline void mux_init(void) {
    for (uint8_t pin = 0; pin < (sizeof(mux_en_pins) / mux_en_pins[0]); pin++) {
        gpio_set_pin_output(mux_en_pins[pin]);
        gpio_write_pin_high(mux_en_pins[pin]);
    }
    for (uint8_t pin = 0; pin < (sizeof(mux_sel_pins) / mux_sel_pins[0]); pin++) {
        gpio_set_pin_output(mux_sel_pins[pin]);
        gpio_write_pin_low(mux_sel_pins[pin]);
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

// Они же active low ?
static inline void mux_disable_unused(uint8_t current_mux) {
    for (uint8_t i = 0; i < MUX_COUNT; i++) {
        if (i != current_mux) {
            gpio_write_pin_high(mux_en_pins[current_mux]);
        }
        gpio_write_pin_low(mux_en_pins[current_mux]);
    }
}

static inline void mux_enable_current(uint8_t current_mux) {}

static inline void mux_channel_select(uint8_t mux, uint8_t channel) {}

// Сканирование конкретного датчика по адресу в матрице
static inline uint16_t ec_sw_scan(uint8_t mux, uint8_t col_physical, uint8_t row) {
    uint16_t raw_adc_readings = 0;

    mux_channel_select(mux, col_physical);

    gpio_write_pin_low(row_pins[row]);

    gpio_set_pin_input(DISCHARGE_PIN);

    gpio_write_pin_high(row_pins[row]);

    raw_adc_readings = analogReadPin(ANALOG_READINGS_INPUT); // Возможно стоит сделать атомарный блок для сканирования + еще поработать над логикой для минимизации шума

    gpio_write_pin_low(DISCHARGE_PIN);
    gpio_set_pin_output(DISCHARGE_PIN);

    wait_us(DISCHARGE_TIME_US);

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

    for (uint8_t count = 0; count < FLOOR_LEVEL_SAMPLING_COUNT; count++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                raw_adc_readings = ec_sw_scan(1, col, row);
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
    for (uint8_t mux = 0; mux < MUX_COUNT; mux++) {
        mux_disable_unused(mux);
        // mux_enable_current();
        uint8_t col_offset = 0;
        for (uint8_t col_physical = 0; col_physical < mux_current_capacity[mux]; col_physical++) {
            for (uint8_t i = 0; i < mux; i++) { // Тут делаем смещение колонки относительно стоящих перед ней в матрице (0 мультиплексор скип)
                col_offset += mux_current_capacity[i];
            }
            uint8_t col_matrix = col_physical + col_offset;

            for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
#ifdef UNUSED_ADRESSES
                if (matrix_address_unused(col_matrix, row)) {
                    continue;
                }
#endif
                uint16_t raw_adc_readings = ec_sw_scan(mux, col_physical, row);

                switch (runtime_config.kb_current_operation_mode) {
                    // Нормальная работа
                    case 0:
                        log_matrix[col_matrix][row] = raw_adc_readings; // Логирования сканирований

                        if (raw_adc_readings <= runtime_config.floor_level_per_key[col_matrix][row]) {
                            break;
                        }

                        uint8_t key_previous_state = (current_matrix[row] >> col_matrix) & 1; // Запрашиваем текущее состояние клавиши (нажата или отпущена)

                        if (raw_adc_readings > runtime_config.actuation_level_per_key[col_matrix][row] && key_previous_state == 0) {
                            current_matrix[row] |= (1 << col_matrix); // Нажимаем
                            has_changed = true;
                        }

                        if (raw_adc_readings < runtime_config.release_level_per_key[col_matrix][row] && key_previous_state == 1) {
                            current_matrix[row] &= ~(1 << col_matrix); // Отпускаем
                            has_changed = true;
                        }

                        break;

                    // Калибровка порогов
                    case 1:
                        uint8_t key_calibration_status = (runtime_config.calibration_status_per_key_bits[row] >> col_matrix) & 1; // Запрашиваем статус калибровки конкретной клавиши

                        if (key_calibration_status == 0) {
                            runtime_config.ceiling_level_per_key[col_matrix][row] = 0;
                            runtime_config.ceiling_level_per_key[col_matrix][row] = raw_adc_readings;
                            runtime_config.calibration_status_per_key_bits[row] |= (1 << col_matrix);
                        }

                        if (runtime_config.ceiling_level_per_key[col_matrix][row] < raw_adc_readings) {
                            runtime_config.ceiling_level_per_key[col_matrix][row] = raw_adc_readings;
                        }

                        break;

                    // Запись SOCD
                    case 2:

                        if (raw_adc_readings < runtime_config.actuation_level_per_key[col_matrix][row]) {
                            break;
                        }
                        socd_mapper(col_matrix, row);
                        break;

                    default:
                        break;
                }
            }
        }
    }

    runtime_config.socd_pair_0_flags_bits = socd_perform_pair(current_matrix, &runtime_config.socd_pair_0, runtime_config.socd_pair_0_flags_bits);
    runtime_config.socd_pair_1_flags_bits = socd_perform_pair(current_matrix, &runtime_config.socd_pair_1, runtime_config.socd_pair_1_flags_bits);
    runtime_config.socd_pair_2_flags_bits = socd_perform_pair(current_matrix, &runtime_config.socd_pair_2, runtime_config.socd_pair_2_flags_bits);

    return has_changed;
}

// Инициализация матрицы СТАНДАРТНАЯ
void matrix_init_custom(void) {
    adc_int();
    pins_init();
    mux_init();
    ec_floor_sample();
}

// Сканирование матрицы СТАНДАРТНАЯ
bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    bool matrix_has_changed = ec_matrix_scan(current_matrix);

    logger();
    return matrix_has_changed;
}