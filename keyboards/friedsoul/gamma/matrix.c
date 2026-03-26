
#include "stdbool.h"
#include "quantum.h"
#include "config.h"
#include "gpio.h"
#include "analog.h"
#include <print.h>
#include "eeprom_config.h"

static const pin_t   row_pins[]                                                 = MATRIX_ROW_PINS;
static const pin_t   mux_en_pins[]                                              = MUX_EN_PINS;
static const pin_t   mux_sel_pins[]                                             = MUX_SEL_PINS;
static const uint8_t mux_channels_in_logical_order[MUX_COUNT][MUX_MAX_CAPACITY] = {MUX_CHANNELS_IN_LOGICAL_ORDER};
static const uint8_t mux_current_capacity[MUX_COUNT]                            = MUX_CURRENT_CAPACITY;

// col_matrix - номер столбца с примененным оффсетом, как они располлагаются в current_matrix
// col_logical - номер столбца в рамках одного мультиплексора, без применения оффсета
// channel - физический номер канала мультиплексора

#ifdef UNUSED_ADRESSES
static const uint8_t matrix_unused_adresses[][2] = UNUSED_ADRESSES;
static inline bool   matrix_address_unused(uint8_t col_matrix, uint8_t row) {
    for (uint8_t i = 0; i < (sizeof(matrix_unused_adresses) / sizeof(matrix_unused_adresses[0])); i++) {
        if (matrix_unused_adresses[i][0] == col_matrix && matrix_unused_adresses[i][1] == row) {
            return true;
        }
    }
    return false;
}
#endif

eeprom_config_t  eeprom_config;
runtime_config_t runtime_config;

// Настройка аналогового пина с помощью PAL
static inline void adc_init(void) {
    palSetLineMode(ANALOG_READINGS_INPUT, PAL_MODE_INPUT_ANALOG);
}

// Пин рязрядки и ряды в output low
static inline void pins_init(void) {
    gpio_write_pin_low(DISCHARGE_PIN);
    gpio_set_pin_output(DISCHARGE_PIN);

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        gpio_write_pin_low(row_pins[row]);
        gpio_set_pin_output(row_pins[row]);
    }
}

// Выставляем управляющие пины мультиплексора
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
static inline void ec_sw_charge(uint8_t row) {
    gpio_write_pin_high(row_pins[row]);
    gpio_set_pin_input(DISCHARGE_PIN);
}

// Разрядка COM линии после сканирования
static inline void ec_sw_discharge(uint8_t row) {
    gpio_write_pin_low(row_pins[row]);
    gpio_write_pin_low(DISCHARGE_PIN);
    gpio_set_pin_output(DISCHARGE_PIN);

    wait_us(DISCHARGE_TIME_US);
}

// Включает выбранный мультиплексор, отключает остальные
static inline void mux_enable_current(uint8_t current_mux) {
    gpio_write_pin_low(mux_en_pins[current_mux]);
}

// Отключается все мультиплексоры
static inline void mux_disable_all(void) {
    for (uint8_t i = 0; i < MUX_COUNT; i++) {
        gpio_write_pin_high(mux_en_pins[i]);
    }
}

// Переключает канал мультиплексора на основе mux и col_logical
static inline void mux_channel_select(uint8_t mux, uint8_t col_logical) {
    uint8_t channel = mux_channels_in_logical_order[mux][col_logical];
    mux_disable_all();

    for (uint8_t pin = 0; pin < (sizeof(mux_sel_pins) / sizeof(mux_sel_pins[0])); pin++) {
        gpio_write_pin(mux_sel_pins[pin], (channel >> pin) & 1);
    }

    mux_enable_current(mux_en_pins[mux]);
    wait_us(5);
}

// Сканирование конкретного датчика по адресу в матрице
static inline uint16_t ec_sw_scan(uint8_t row) {
    uint16_t raw_adc_readings = 0;

    gpio_write_pin_low(row_pins[row]);
    gpio_set_pin_input(DISCHARGE_PIN);

    gpio_write_pin_high(row_pins[row]);
    raw_adc_readings = analogReadPin(ANALOG_READINGS_INPUT); // Возможно стоит сделать атомарный блок для сканирования + еще поработать над логикой для минимизации шума

    gpio_write_pin_low(DISCHARGE_PIN);
    gpio_set_pin_output(DISCHARGE_PIN);

    wait_us(DISCHARGE_TIME_US);
    uprintf("Row %d: %d\n", row, raw_adc_readings);
    return raw_adc_readings;
}

// Семплинг нижнего порога клавиш во время старта.
static inline void ec_floor_sample(void) {
    uint16_t raw_adc_readings = 0;
    memset(&runtime_config.floor_level_per_key, 0, sizeof(runtime_config.floor_level_per_key)); // Заполняем массив нулями

    for (uint8_t count = 0; count < FLOOR_LEVEL_SAMPLING_COUNT; count++) {
        for (uint8_t mux = 0; mux < MUX_COUNT; mux++) {
            for (uint8_t col_logical = 0; col_logical < mux_current_capacity[mux]; col_logical++) {
                // uint8_t col_matrix = col_logical;
                uint8_t col_offset = 0;
                for (uint8_t i = 0; i < mux; i++) {
                    // col_matrix += mux_current_capacity[i]; // Вычисляем смещение для col_matrix на основе текущей емкости мультиплексора (Для 0 мультиплексора смещения нет)
                    col_offset += mux_current_capacity[i];
                }

                uint8_t col_matrix = col_logical + col_offset;
                mux_channel_select(mux, col_logical); // Переключаем канал мультиплексора

                for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
#ifdef UNUSED_ADRESSES
                    if (matrix_address_unused(col_matrix, row)) {
                        continue;
                    }
#endif
                    raw_adc_readings = ec_sw_scan(row);
                    if (raw_adc_readings > runtime_config.floor_level_per_key[col_matrix][row]) {
                        runtime_config.floor_level_per_key[col_matrix][row] = raw_adc_readings;
                    }
                }
            }
        }
    }
}

// Функция сканирования и обновления current matrix
bool ec_matrix_scan(matrix_row_t current_matrix[]) {
    bool     has_changed      = false;
    uint16_t raw_adc_readings = 0;
    for (uint8_t mux = 0; mux < MUX_COUNT; mux++) {
        for (uint8_t col_logical = 0; col_logical < mux_current_capacity[mux]; col_logical++) {
            uint8_t col_matrix = col_logical;
            for (uint8_t i = 0; i < mux; i++) {
                col_matrix += mux_current_capacity[i]; // Вычисляем смещение для col_matrix на основе текущей емкости мультиплексора (Для 0 мультиплексора смещения нет)
            }

            mux_channel_select(mux, col_logical); // Переключаем канал мультиплексора

            for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
#ifdef UNUSED_ADRESSES
                if (matrix_address_unused(col_matrix, row)) {
                    continue;
                }
#endif
                raw_adc_readings            = ec_sw_scan(row);
                log_matrix[col_matrix][row] = raw_adc_readings; // Логирования сканирований

                switch (runtime_config.kb_current_operation_mode) {
                    // Нормальная работа
                    case 0:
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
    adc_init();
    pins_init();
    mux_init();
    // ec_floor_sample();
}

// Сканирование матрицы СТАНДАРТНАЯ
bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    bool matrix_has_changed = ec_matrix_scan(current_matrix);

    logger();
    return matrix_has_changed;
}