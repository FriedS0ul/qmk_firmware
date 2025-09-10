

#include <matrix.h>
#include "config.h"
#include <drivers/analog.h>
//#include <avr/iom32u4.h>
#include <avr/io.h>
#include <quantum.h>
#include <gpio.h>
#include <print.h>



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

    
}


//Matrix scanning process
bool matrix_scan_custom(matrix_row_t current_matrix[]){
    bool matrix_has_changed = true;

    gpio_write_pin_low(AMUX_EN_PINS); // Включаем мультиплексор и выставляем каналы в 0 0 0

    gpio_write_pin_low(F4);
    gpio_write_pin_low(F5);
    gpio_write_pin_low(D4);
    
    wait_us(10);

    gpio_write_pin_high(D4); //Включаем зарядку ряда
    wait_us(100);
    gpio_write_pin_low(D4); // Отключаем зарядку ряда

    wait_us(10);

    ADCSRA |= (1 << ADSC);// Включение чтения ацп
    while (ADCSRA & (1 << ADSC)){

        // Ждем автоматического завершения чтения

    }
    //uint16_t result = (ADCL) | (ADCH << 8);
    uint16_t adc_readings = ADC;

    uprintf("%u\n", adc_readings);

    return matrix_has_changed;

}
