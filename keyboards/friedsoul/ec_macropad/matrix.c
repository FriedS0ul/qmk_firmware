
#include <quantum.h>
#include "config.h"
#include <matrix.h>
#include <avr/io.h>
#include <gpio.h>
#include <print.h>
#include <analog.h>


const pin_t row_pins[] = MATRIX_ROWS_PINS;
const pin_t amux_sel[] = AMUX_SEL_PINS;
uint16_t ec_noise_threshold[MATRIX_COLS][MATRIX_ROWS];
uint16_t log_matrix[MATRIX_COLS][MATRIX_ROWS];
uint8_t scan_counter = 0;


// Инициализация АЦП
void adc_init(void){
    //analogReference(ADC_REF_POWER);
    ADMUX |= (1 << REFS0) | (1 << REFS1) ; // Опорное напряжение == Vcc
    //ADCSRA |= (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2); // Делитель 128 для установки частоты ацп 125Кгц
    ADCSRA |= (1 << ADPS1) | (1 << ADPS2); // Делитель 64
    ADCSRA &= ~(1 << ADATE); // Auto trigger выкл
    ADCSRA |= (1 << ADEN); // Включение АЦП
    DIDR0 |= (1 << ADC0D); // Отключение цифрогово входа на входном пине F0
}

// Инициализация пинов
void pins_init(void){

    gpio_set_pin_input(ANALOG_READINGS_INPUT); 
    gpio_set_pin_output(AMUX_EN_PINS); 
    gpio_set_pin_input(DISCHARGE_PIN); 

    for (uint8_t i = 0; i < (sizeof(row_pins) / sizeof(row_pins[0])); i++) // Выставляем ряды матрицы как output low
    {
        gpio_set_pin_output(row_pins[i]);
        gpio_write_pin_low(row_pins[i]);
    }

    for (uint8_t i = 0; i < (sizeof(amux_sel) / sizeof(amux_sel[0])); i++) // Выставляем управлящие пины мультиплексора как ouput low
    {
        gpio_set_pin_output(amux_sel[i]);
        gpio_write_pin_low(amux_sel[i]);
    }
}

// Функция вывода лога с данными сканирования датчиков каждые 10 циклов сканирования
void log_print(void)
{
    if (scan_counter == 70)
    {
       for (uint8_t col = 0; col < MATRIX_COLS; col++)
        {
            for (uint8_t row = 0; row < MATRIX_ROWS; row++)
            {
            uprintf("COl %d ROW %d: %u", col, row, log_matrix[col][row]);
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

// Функция зарядки ряда
void row_charge(uint8_t pin)
{
    gpio_set_pin_input(DISCHARGE_PIN);
    gpio_write_pin_high(row_pins[pin]);
    wait_us(CHARGE_TIME_US);
}

// Функция разрядки аналогового пина
void com_discharge(void)
{
    gpio_set_pin_output(DISCHARGE_PIN);
    gpio_write_pin_low(DISCHARGE_PIN);
    wait_us(DISCHARGE_TIME_US);
    gpio_set_pin_input(DISCHARGE_PIN);
}

// Функция для выбора каналов мультиплексора
void mux_channel_select(uint8_t col) 
{
    gpio_write_pin_high(AMUX_EN_PINS);
    for (uint8_t i = 0; i < (sizeof(amux_sel) / sizeof(amux_sel[0])); i++) // Выбираем значение low или high на основе номера нужной колонки
    {
        gpio_write_pin(amux_sel[i], (col >> i) & 1);
    }
    gpio_write_pin_low(AMUX_EN_PINS);
    wait_us(5);
}

void rows_low(void)
{
    for (uint8_t row = 0; row < MATRIX_ROWS; row++)
    {
        gpio_set_pin_output(row_pins[row]);
        gpio_write_pin_low(row_pins[row]);
    } 
    
}

// Сканирование RAW значений с конкретного датчика по адресу в матрице
uint16_t ec_sw_scan_raw(uint8_t row)
{
    uint16_t raw_adc_readings = 0;

    gpio_write_pin_high(row_pins[row]);
    wait_us(CHARGE_TIME_US);

    cli();

    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC)){} // dummy conversion

    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC)){} // Чтение

    raw_adc_readings = ADC;

    sei();

    rows_low();
    com_discharge();

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
                ec_noise_threshold[col][row] += ec_sw_scan_raw(row);
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
            uint16_t raw_adc_readings = ec_sw_scan_raw(row);
            uint8_t previous_state = (current_matrix[row] >> col) & 1; // Запрос текущего стостояния (нажата или отпущена)
            log_matrix[col][row] = raw_adc_readings; // Логирование нажатий в массив

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
    return matrix_has_changed;
}

// Инициализация при старте (СТАНДАРТНАЯ ФУНКЦИЯ)
void matrix_init_custom(void){
    
    pins_init();
    adc_init();
    //ec_matrix_noise_sample();
}


// Скан матрицы (СТАНДАРТНАЯ ФУНКЦИЯ)
bool matrix_scan_custom(matrix_row_t current_matrix[]){
    bool matrix_has_changed = ec_matrix_scan(current_matrix);

    log_print();
    
    return matrix_has_changed;
}
