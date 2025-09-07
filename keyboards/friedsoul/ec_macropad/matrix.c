

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
    DIDR0 |= (1 << ADC0D); // Отключение цифрогово входа на входном пине (F0)
    ADCSRA |= (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2); // Делитель 128 для установки частоты ацп 125Кгц
    ADCSRA |= (1 << ADEN); // Включение АЦП

}


// Initialize hardware here
void matrix_init_custom(void){
    adc_init();

    gpio_write_pin_low(AMUX_EN_PINS);
    gpio_write_pin_low(F4);
    gpio_write_pin_low(F5);
    gpio_write_pin_low(F6);





}


//Matrix scanning process
bool matrix_scan_custom(matrix_row_t current_matrix[]){




    return matrix_has_changed;
}
