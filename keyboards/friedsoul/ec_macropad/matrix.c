
#include <quantum.h>
#include "config.h"
#include <matrix.h>
#include <avr/io.h>
#include <gpio.h>
#include <print.h>
#include <analog.h>


const pin_t row_pins[] = MATRIX_ROWS_PINS;
const pin_t amux_sel[] = AMUX_SEL_PINS;
uint16_t ec_noise_threshold[MATRIX_ROWS][MATRIX_COLS];


// Инициализация АЦП
void adc_init(void){
    analogReference(ADC_REF_POWER);
    //ADMUX = (1 << REFS0); // Опорное напряжение == Vcc
    ADCSRA |= (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2); // Делитель 128 для установки частоты ацп 125Кгц
    ADCSRA |= (1 << ADEN); // Включение АЦП
    DIDR0 |= (1 << ADC0D); // Отключение цифрогово входа на входном пине F0
}

// Инициализация пинов
void pins_init(void){

    gpio_set_pin_input(ANALOG_READINGS_INPUT); // Аналоговый пин становится входом
    gpio_set_pin_output(AMUX_EN_PINS); // AMUX_EN становится выходом
    gpio_write_pin_low(DISCHARGE_PIN); // Инициализация пина для разрядки ряда
    gpio_set_pin_output(DISCHARGE_PIN); // Инициализация пина для разрядки ряда

    for (uint8_t i = 0; i < (sizeof(row_pins) / sizeof(row_pins[0])); i++) // Выставляем управлящие пины ряды матрицы как выходы + low
    {
        gpio_set_pin_output(row_pins[i]);
        gpio_write_pin_high(row_pins[i]); 
    }

    for (uint8_t i = 0; i < (sizeof(amux_sel) / sizeof(amux_sel[0])); i++) // Выставляем управлящие пины мультиплексора как выходы + low
    {
        gpio_set_pin_output(amux_sel[i]);
        gpio_write_pin_low(amux_sel[i]);
    }
}

// Функция зарядки ряда
void row_charge(uint8_t pin)
{
    gpio_set_pin_input(DISCHARGE_PIN);
    gpio_write_pin_high(row_pins[pin]);
    wait_us(CHARGE_TIME_US);
}

// Функция разрядки аналогового пина
void pin_discharge(void)
{
    gpio_write_pin_low(DISCHARGE_PIN); 
    gpio_set_pin_output(DISCHARGE_PIN); 
    wait_us(DISCHARGE_TIME_US); 
}

// Функция для выбора каналов мультиплексора
void mux_channel_select(uint8_t col) 
{
    gpio_write_pin_high(AMUX_EN_PINS);
    for (uint8_t i = 0; i < (sizeof(amux_sel) / sizeof(amux_sel[0])); i++) // Выбираем значение low или high на основе номера нужной колонки
    {
        gpio_write_pin(amux_sel[i], (col >> i) & 1);
    }
}

// Сканирование RAW значений с конкретного датчика по адресу в матрице
uint16_t ec_sw_scan_raw(uint8_t col, uint8_t row)
{

    gpio_set_pin_input(DISCHARGE_PIN);



    gpio_write_pin_low(AMUX_EN_PINS);
    wait_us(30); // MUX вкл


    uint16_t raw_adc_readings = analogReadPin(ANALOG_READINGS_INPUT);


    gpio_write_pin_high(AMUX_EN_PINS); // MUX выкл
    gpio_write_pin_low(DISCHARGE_PIN);
    gpio_set_pin_output(DISCHARGE_PIN);

    uprintf("ROW %d, COL %d: %u\r\n", row, col, raw_adc_readings); // Выводим полученные значения в HID консоль

    wait_ms(5);

    return raw_adc_readings;
}

// Семплинг шума матрицы при инициализации
void ec_matrix_noise_sample(void)
{
    for (uint8_t col = 0; col < MATRIX_COLS; col++) // Заполняем таблицу нулями
    {
        for (uint8_t row = 0; row < MATRIX_ROWS; row++)
        {
            ec_noise_threshold[col][row] = 0;
        } 
    }
    
    for (uint8_t sample = 0; sample < NOISE_THRESHOLD_SAMPLING_COUNT; sample++) // Производим семплинг и записываем значения
    {
        for (uint8_t col = 0; col < MATRIX_COLS; col++)
        {
            mux_channel_select(col);
            for (uint8_t row = 0; row < MATRIX_ROWS; row++)
            {
                ec_noise_threshold[col][row] += ec_sw_scan_raw(col, row);
            }
        }
    }

    for (uint8_t col = 0; col < MATRIX_COLS; col++) // Усредняем значения и добавляем зазор
    {
        for (uint8_t row = 0; row < MATRIX_ROWS; row++)
        {
            ec_noise_threshold[col][row] /= NOISE_THRESHOLD_SAMPLING_COUNT;
            ec_noise_threshold[col][row] += NOISE_OFFSET;
        } 
    }
     uprintf("Sampling done 0,0 threshold: %u\n", ec_noise_threshold[0][0]);
}

//Сканирование матрицы и обновление current_matrix
bool ec_matrix_scan(matrix_row_t current_matrix[])
{
    bool matrix_has_changed = false;
    for (uint8_t col = 0; col < MATRIX_COLS; col++)
    {
        mux_channel_select(col);
        for (uint8_t row = 0; row < MATRIX_ROWS; row++)
        {
            uint16_t raw_adc_readings = ec_sw_scan_raw(col, row);
            uint8_t previous_state = (current_matrix[row] >> col) & 1;

            if (raw_adc_readings < RELEASE_LEVEL && previous_state == 1) 
            {
                current_matrix[row] &= ~(1 << col); // Отпускаем
                matrix_has_changed = true;
            }
            else if (raw_adc_readings > ACTUATION_LEVEL && previous_state == 0)
            {
                current_matrix[row] |= (1 << col); // Нажимаем
                matrix_has_changed = true;
            }
        }
    }
    wait_ms(200); // Cнижаем частоту опроса для тестов
    return matrix_has_changed;
}

// Инициализация при старте (СТАНДАРТНАЯ ФУНКЦИЯ)
void matrix_init_custom(void){
    
    pins_init();
    adc_init();
    ec_matrix_noise_sample();
}


// Скан матрицы (СТАНДАРТНАЯ ФУНКЦИЯ)
bool matrix_scan_custom(matrix_row_t current_matrix[]){
    bool matrix_has_changed = ec_matrix_scan(current_matrix);

    return matrix_has_changed;
}
