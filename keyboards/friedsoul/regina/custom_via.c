
#include "via.h"

// Переменные - ID для дополнительных меню в VIA
enum via_extras_ids {

    id_calibration_onoff = 1,
    id_console_log_onoff = 2

};

void via_custom_config_get_value(){

}

void via_custom_config_set_value(){

}

void via_custom_config_save(){

}

void via_custom_value_command_kb(uint8_t *data, uint8_t lenght) {
    // 8-bit | data = [ command_id, channel_id, value_id, value_data ]
    // 16-bit | data = [ value_id, value_data ]
    uint8_t *command_id        = &data[0];
    uint8_t *channel_id        = &data[1];
    uint8_t *value_id_and_data = &data[3];

    if (*channel_id == id_custom_channel){
        switch (*command_id) {

        case id_custom_set_value:

            via_custom_config_set_value();

            break;
        
        case id_custom_get_value:

            via_custom_config_get_value();

            break;
            
        case id_custom_save:
            
            via_custom_config_save();

            break;
        
        default:
            // Ошибка
            *command_id = id_unhandled;
            break;
        }
    }
}