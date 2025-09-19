
#include <quantum.h>
#include "config.h"
#include <matrix.h>
//#include <avr/iom32u4.h>
#include <avr/io.h>
#include <gpio.h>
#include <print.h>
#include <analog.h>


const pin_t row_pins[] = MATRIX_ROWS_PINS;
const pin_t amux_sel[] = AMUX_SEL_PINS;


// Инициализация АЦП
void adc_init(void){

    ADMUX = (1 << REFS0); // Опорное напряжение == Vcc
    ADCSRA |= (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2); // Делитель 128 для установки частоты ацп 125Кгц
    ADCSRA |= (1 << ADEN); // Включение АЦП
    DIDR0 |= (1 << ADC0D); // Отключение цифрогово входа на входном пине F0
}

// Инициализация пинов (вход - выход)
void pins_init(void){

    gpio_set_pin_input(ANALOG_READINGS_INPUT); // Аналоговый пин становится входом
    gpio_set_pin_output(AMUX_EN_PINS); // AMUX_EN становится выходом
    gpio_write_pin_low(DISCHARGE_PIN); // Инициализация пина для разрядки ряда
    gpio_set_pin_output(DISCHARGE_PIN); // Инициализация пина для разрядки ряда

    for (uint8_t i = 0; i < (sizeof(row_pins) / sizeof(row_pins[0])); i++) // Выставляем управлящие пины ряды матрицы как выходы + low
    {
        gpio_set_pin_output(row_pins[i]);
        gpio_write_pin_low(row_pins[i]); 
    }

    for (uint8_t i = 0; i < (sizeof(amux_sel) / sizeof(amux_sel[0])); i++) // Выставляем управлящие пины мультиплексора как выходы + low
    {
        gpio_set_pin_output(amux_sel[i]);
        gpio_write_pin_low(amux_sel[i]);
    }
}

// Функция зарядки ряда
void row_charge(pin_t pin){

    gpio_set_pin_input(DISCHARGE_PIN); // Отключаем пин разрядки

    gpio_write_pin_high(pin); // Включаем зарядку
    wait_us(CHARGE_TIME_US); // Ждем зарядку
}

// Функция разрядки ряда
void pin_discharge(void)
{
    gpio_write_pin_low(DISCHARGE_PIN); // Разрядный пин в low
    gpio_set_pin_output(DISCHARGE_PIN); // Включаем разрядку
    wait_us(DISCHARGE_TIME_US); // Ждем разрядки
}

// Функция для выбора каналов мультиплексора
void mux_channel_select(uint8_t col) 
{
    gpio_write_pin_high(AMUX_EN_PINS); // MUX выкл

    for (uint8_t i = 0; i < (sizeof(amux_sel) / sizeof(amux_sel[0])); i++) // Выбираем значение low или high на основе номера нужной колонки
    {
        gpio_write_pin(amux_sel[i], (col >> i) & 1);
    }
    wait_us(10); // Ждем на всякий случай чтобы точно переключилось
    gpio_write_pin_low(AMUX_EN_PINS); // MUX вкл
}

// Инициализация при старте (СТАНДАРТНАЯ ФУНКЦИЯ)
void matrix_init_custom(void){
    
    pins_init();
    adc_init();
}


// Скан матрицы (СТАНДАРТНАЯ ФУНКЦИЯ)
bool matrix_scan_custom(matrix_row_t current_matrix[]){
    bool matrix_has_changed = true;

    for (uint8_t col = 0; col < (sizeof(amux_sel) / sizeof(amux_sel[0])); col++)
    {
        mux_channel_select(col);

        for (uint8_t row = 0; row < (sizeof(row_pins) / sizeof(row_pins[0])); row++)
        {
            gpio_write_pin_low(row_pins[row]);
            wait_us(DISCHARGE_TIME_US); // Разряжаем ряд

            row_charge(row_pins[row]);

            uint16_t raw_adc_readings = analogReadPin(ANALOG_READINGS_INPUT); // Читаем и записываем в переменную
            uprintf("Row %d, Col %d: %u\r\n", row, col, raw_adc_readings); // Выводим полученные значения в HID консоль

            pin_discharge(); // Разряжаем отрезок от мультиплектора до контроллера !!! НЕ РЯД !!!
        }
    }
    wait_ms(100); // Cнижаем частоту опроса для тестов
    return matrix_has_changed;
}
