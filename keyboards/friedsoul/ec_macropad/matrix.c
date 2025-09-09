

#include <matrix.h>
#include "config.h"
#include <drivers/analog.h>
#include <avr/io.h>
#include <avr/iom32u4.h>
#include <quantum.h>
#include <gpio.h>


extern matrix_row_t raw_matrix[MATRIX_ROWS];
extern matrix_row_t matrix[MATRIX_ROWS];

// Инициализация АЦП
void adc_init(void){

    ADMUX = (1 << REFS0); // Опорное напряжение == Vcc
    ADCSRA |= (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2); // Делитель 128 для установки частоты ацп 125Кгц
    ADCSRA |= (1 << ADEN); // Включение АЦП

}

// Инициализация пинов (вход - выход)
void pins_init(void){

    DIDR0 |= (1 << ADC0D); // Отключение цифрогово входа на входном пине F0
    DDRF &= ~(1 << DDF0); // Сброс значения F0 (становится входом)
    DDRF |= (1 << DDF4) | (1 << DDF5) | (1 << DDF6); // Установка упрявляющих пинов мультиплексора как выходов
    DDRD = (1 << DDD4) | (1 << DDD6) | (1 << DDD7); // Установка рядов матрицы как выходов (0, 1, 2)

}


// Initialize hardware here
void matrix_init_custom(void){

    pins_init();
    adc_init();

    gpio_write_pin_low(AMUX_EN_PINS);
    gpio_write_pin_low(F4);
    gpio_write_pin_low(F5);
    gpio_write_pin_low(F6);





}


//Matrix scanning process
bool matrix_scan_custom(matrix_row_t current_matrix[]){
    
    bool matrix_has_changed = false;




    return matrix_has_changed;
}
