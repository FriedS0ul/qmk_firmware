
#include "via.h"
#include "config.h"
#include "eeprom_config.h"



// Переменные - ID для дополнительных элементов меню в VIA
enum via_extras_value_ids {

    id_console_log_onoff = 1,
    id_calibration_onoff = 2
    
};

// VIA Заправшивает данные от клавиатуры
void via_custom_config_kb_to_via(uint8_t *data){
    // data = [ value_id, value_data ]
    uint8_t *value_id = &(data[0]);
    uint8_t *value_data = &(data[1]);

    switch (*value_id)
    {
    case id_console_log_onoff:

        *value_data == eeprom_config.console_log_status;

        break;

    case id_calibration_onoff:
        
        *value_data == runtime_config.calibration_status;
        break;
    
    default:
        // Ничего
        break;
    }
}

void via_custom_config_kb_from_via(uint8_t *data){
    // data = [ value_id, value_data ]
    uint8_t *value_id = &(data[0]);
    uint8_t *value_data = &(data[1]);

    switch (*value_id)
    {
    case id_console_log_onoff:
        /* code */
        break;

    case id_calibration_onoff:
        /* code */
        break;
    
    default:
        // Ничего
        break;
    }
}

void via_custom_config_save(uint8_t *data){

}

// Функция обмена данными между клавиатурой и VIA + сохранение данных в eeprom
void via_custom_value_command_kb(uint8_t *data, uint8_t lenght) {
    // data = [ command_id, channel_id, value_id, value_data ]
    uint8_t *command_id        = &(data[0]);
    uint8_t *channel_id        = &(data[1]);
    uint8_t *value_id_and_data = &(data[3]);

    if (*channel_id == id_custom_channel){

        switch (*command_id) {

        case id_custom_set_value:

            via_custom_config_kb_from_via(value_id_and_data);

            break;
        
        case id_custom_get_value:

            via_custom_config_kb_to_via(value_id_and_data);

            break;
            
        case id_custom_save:
            
            break;
        
        default:
            // Ошибка
            *command_id = id_unhandled;
            break;
        }
    }

    // Ошибка
    *command_id = id_unhandled;
}