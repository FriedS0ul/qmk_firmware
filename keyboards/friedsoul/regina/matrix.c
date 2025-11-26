
#include "stdbool.h"
#include "quantum.h"
#include "config.h"
#include "gpio.h"

const pin_t row_pins [] = MATRIX_ROW_PINS;


void adc_int(void){

    palSetLineMode(ANALOG_READINGS_INPUT, PAL_MODE_INPUT_ANALOG);

}

void pins_init(void){

    gpio_set_pin_input_low(DISCHARGE_PIN);

    for (size_t i = 0; i < MATRIX_ROWS; i++)
    {
        gpio_set_pin_output(row_pins[i]);
        gpio_write_pin_low(row_pins[i]);
    }
    
}








void matrix_init_custom(void) {
    
    adc_int();
    pins_init();
    
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    bool matrix_has_changed = true;


    return matrix_has_changed;
}