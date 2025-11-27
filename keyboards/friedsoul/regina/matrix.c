
#include "stdbool.h"
#include "quantum.h"
#include "config.h"
#include "gpio.h"
#include "analog.h"
#include <print.h>

const pin_t row_pins [] = MATRIX_ROW_PINS;
uint16_t log_matrix[MATRIX_COLS][MATRIX_ROWS];
uint8_t scan_counter = 0;


void adc_int(void){

    palSetLineMode(ANALOG_READINGS_INPUT, PAL_MODE_INPUT_ANALOG);

}

void pins_init(void){

    gpio_set_pin_input(ANALOG_READINGS_INPUT);

    gpio_write_pin_low(DISCHARGE_PIN);
    gpio_set_pin_output(DISCHARGE_PIN);
    

    for (uint8_t i = 0; i < MATRIX_ROWS; i++)
    {
        gpio_write_pin_high(row_pins[i]);
        gpio_set_pin_output(row_pins[i]);
        
    }
    
}
 // Вывод сканирования в консоль каждые 250 циклов
 void logger(void){
    if (scan_counter == 250)
    {
        uprintf("\n");
        uprintf("\n");
        for (uint8_t col = 0; col < MATRIX_COLS; col++)
        {
            for (uint8_t row = 0; row < MATRIX_ROWS; row++)
            {
                uprintf("COL %d ROW %d: %u", col, row, log_matrix[col][row]);
                uprintf("\n");
            }
        }
        scan_counter = 0;
    }
    else
    {
        scan_counter++;
    }
 }

 // Сканирование конкретного датчика по адресу в матрице
 uint16_t ec_sw_scan(pin_t col, pin_t row){

    uint16_t raw_adc_readings = 0;

    gpio_write_pin_low(DISCHARGE_PIN);
    gpio_set_pin_input(DISCHARGE_PIN);

    gpio_write_pin_low(row);
    gpio_set_pin_output(row);

    ATOMIC_BLOCK_FORCEON
    {
        gpio_write_pin_high(row);
        gpio_set_pin_output(row);

        raw_adc_readings = analogReadPin(ANALOG_READINGS_INPUT);
    }

    log_matrix[col][row] = raw_adc_readings;

    gpio_write_pin_low(DISCHARGE_PIN);
    gpio_set_pin_output(DISCHARGE_PIN);

    wait_us(50);

    //uprintf("something %u\r\n", raw_adc_readings);

    return raw_adc_readings;
 }



void matrix_init_custom(void) {
    adc_int();
    pins_init();

    uprintf("initialization complete\r\n"); 
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    bool matrix_has_changed = true;

    logger();

    for (uint8_t col = 0; col < MATRIX_COLS; col++)
    {
        for (uint8_t row = 0; row < MATRIX_ROWS; row++)
        {
            ec_sw_scan(col, row);
        }
        
    }
    return matrix_has_changed;
}