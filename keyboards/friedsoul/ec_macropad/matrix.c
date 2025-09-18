

#include <matrix.h>
#include "config.h"
#include <drivers/analog.h>
//#include <avr/iom32u4.h>
#include <avr/io.h>
#include <quantum.h>
#include <gpio.h>
#include <print.h>




const pin_t row_pins[] = MATRIX_ROWS_PINS;
const pin_t amux_sel[] = AMUX_SEL_PINS;

static adc_mux adcMux;


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
    gpio_write_pin_high(AMUX_EN_PINS); // Выключение мультиплексора
    gpio_write_pin_low(DISCHARGE_PIN); // Инициализация пина для разрядки ряда
    gpio_set_pin_output(DISCHARGE_PIN); // Инициализация пина для разрядки ряда

    for (uint8_t i = 0; i < (sizeof(amux_sel) / sizeof(amux_sel[0])); i++) // Выставляем управлящие пины мультиплексора как выходы + low
    {
        gpio_set_pin_output(amux_sel[i]);
        gpio_write_pin_low(amux_sel[i]);
    }

    for (uint8_t i = 0; i < (sizeof(row_pins) / sizeof(row_pins[0])); i++) // Выставляем управлящие пины ряды матрицы как выходы + low
    {
        gpio_set_pin_output(row_pins[i]);
        gpio_write_pin_low(row_pins[i]); 
    }
}

// Функция зарядки ряда
void row_charge(pin_t ){

    gpio_set_pin_input(DISCHARGE_PIN); // Отключаем пин разрядки
    gpio_write_pin_high(pin); // Включаем зарядку
    wait_us(50);
}

// Функция разрядки ряда
void row_discharge(void){

    gpio_set_pin_output(DISCHARGE_PIN);
    gpio_write_pin_low(DISCHARGE_PIN);
}

// Инициализация при старте (СТАНДАРТНАЯ ФУНКЦИЯ)
void matrix_init_custom(void){
    
    adcMux = pinToMux(ANALOG_READINGS_INPUT);
    pins_init();
    adc_init();
}


// Скан матрицы (СТАНДАРТНАЯ ФУНКЦИЯ)
bool matrix_scan_custom(matrix_row_t current_matrix[]){
    bool matrix_has_changed = true;

    for (uint8_t row = 0; row < (sizeof(row_pins) / sizeof(row_pins[0])); row++)
    {
        row_charge(row_pins[row]); // Включаем зарядку ряда
        gpio_write_pin_low(AMUX_EN_PINS); // Включаем мультиплексор
        
        for (uint8_t col = 0; col < AMUX_CHANNELS_OCCUPIED; col++)
        { 
            gpio_write_pin(F4, (col >> 0) & 1); // Извлекаем бит и устанавливаем пин high или low
            gpio_write_pin(F5, (col >> 1) & 1);
            gpio_write_pin(D4, (col >> 2) & 1);
            
            uint16_t adc_readings = adc_read(adcMux); // Смотрим прочитанное значение

            uprintf("Row %d, Col %d: %u\r\n", row, col, adc_readings); // Выводим полученные значения в HID консоль
        }

        gpio_write_pin_high(AMUX_EN_PINS); // Отключаем мультиплексор 

        row_discharge();

        wait_ms(20); // Снижаем частоту опроса для тестов
    }
    
    return matrix_has_changed;
}
