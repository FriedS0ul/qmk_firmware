

#include <matrix.h>
#include "config.h"
#include <drivers/analog.h>


extern matrix_row_t raw_matrix[MATRIX_ROWS];
extern matrix_row_t matrix[MATRIX_ROWS];

static adc_mux adcMux;


// Initialize hardware here
void matrix_init_custom(void){


    

    adcMux = pinToMux(ANALOG_READINGS_INPUT);
    adc_read(adcMux);





}


//Matrix scanning process
bool matrix_scan_custom(matrix_row_t current_matrix[]){
    bool matrix_has_changed = false;




    return matrix_has_changed;
}
