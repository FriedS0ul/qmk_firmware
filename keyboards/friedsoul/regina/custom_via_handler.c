
#include "via.h"
#include "config.h"
#include "eeprom_config.h"

// Переменные - ID для дополнительных элементов меню в VIA
enum via_extras_value_ids {

    id_save_to_eeprom         = 1,  // button
    id_console_log_status     = 2,  // dropdown
    id_calibration_status     = 3,  // toggle
    id_actuation_level        = 4,  // range
    id_release_level          = 5,  // range
    id_show_kb_reset          = 6,  // toggle
    id_kb_reset               = 7,  // button
    id_show_advanced_features = 8,  // toggle
    id_socd_status_global     = 9,  // toggle
    id_socd_pair_current      = 10, // dropdown
    id_socd_pair_mapping      = 11, // button
    id_socd_pair_0_mode       = 12, // dropdown
    id_socd_pair_1_mode       = 13, // dropdown
    id_socd_pair_2_mode       = 14, // dropdown
    id_actuation_mode_global  = 15  // dropdown

};

// VIA Передает данные клавиатуре
void via_custom_config_via_to_kb(uint8_t *data) {
    // data = [ value_id, value_data ]
    uint8_t *value_id   = &(data[0]);
    uint8_t *value_data = &(data[1]);

    switch (*value_id) {
        case id_console_log_status:
            runtime_config.console_log_status = *value_data;

            break;

        case id_calibration_status:
            runtime_config.kb_current_operation_mode = *value_data;

            break;

        case id_actuation_level:
            runtime_config.actuation_level_global = value_data[0] << 8 | value_data[1];

            break;

        case id_release_level:
            runtime_config.release_level_global = value_data[0] << 8 | value_data[1];

            break;

        case id_show_advanced_features:
            switch (*value_data) {
                case 1:
                    runtime_config.advanced_features_status_bits |= (1 << bits_advanced_features_global);

                    break;

                case 0:
                    runtime_config.advanced_features_status_bits &= ~(1 << bits_advanced_features_global);

                    break;

                default:
                    break;
            }
            break;

        case id_socd_status_global:
            switch (*value_data) {
                case 1:
                    runtime_config.advanced_features_status_bits |= (1 << bits_socd_status_global);

                    break;

                case 0:
                    runtime_config.advanced_features_status_bits &= ~(1 << bits_socd_status_global);

                    break;

                default:
                    break;
            }
            break;

        case id_socd_pair_current:

            runtime_config.socd_pair_current = *value_data;

            break;

        case id_socd_pair_mapping:
            runtime_config.kb_current_operation_mode = *value_data;

            break;

        case id_socd_pair_0_mode:
            runtime_config.socd_pair_0.pair_mode = *value_data;

            break;

        case id_socd_pair_1_mode:
            runtime_config.socd_pair_1.pair_mode = *value_data;

            break;

        case id_socd_pair_2_mode:
            runtime_config.socd_pair_2.pair_mode = *value_data;

            break;

        case id_actuation_mode_global:

            switch (*value_data)
            {
            case 0:
                runtime_config.advanced_features_status_bits &= ~(1 << bits_actuation_mode_global);

                break;
        
            case 1:
                runtime_config.advanced_features_status_bits |= (1 << bits_actuation_mode_global);

                break;
            }
            break;

        case id_show_kb_reset:

            break;

        case id_kb_reset:
            eeprom_reset();
            runtime_renew();

            break;

        case id_save_to_eeprom:
            save_to_eeprom();
            runtime_renew();

            break;

        default:
            // Ничего
            break;
    }
}

// VIA Получает данные от клавиатуры
void via_custom_config_via_from_kb(uint8_t *data) {
    // data = [ value_id, value_data ]
    uint8_t *value_id   = &(data[0]);
    uint8_t *value_data = &(data[1]);

    switch (*value_id) {
        case id_console_log_status:
            *value_data = runtime_config.console_log_status;

            break;

        case id_calibration_status:
            *value_data = runtime_config.kb_current_operation_mode;

            break;

        case id_actuation_level:
            value_data[0] = runtime_config.actuation_level_global >> 8;
            value_data[1] = runtime_config.actuation_level_global & 0xFF;

            break;

        case id_release_level:
            value_data[0] = runtime_config.release_level_global >> 8;
            value_data[1] = runtime_config.release_level_global & 0xFF;

            break;

        case id_show_advanced_features:
            *value_data = (runtime_config.advanced_features_status_bits >> bits_advanced_features_global) & 1;

            break;

        case id_socd_status_global:
            *value_data = (runtime_config.advanced_features_status_bits >> bits_socd_status_global) & 1;

            break;

        case id_socd_pair_current:

            break;

        case id_socd_pair_0_mode:
            *value_data = runtime_config.socd_pair_0.pair_mode;

            break;

        case id_socd_pair_1_mode:
            *value_data = runtime_config.socd_pair_1.pair_mode;

            break;

        case id_socd_pair_2_mode:
            *value_data = runtime_config.socd_pair_2.pair_mode;

            break;

        case id_actuation_mode_global:

            *value_data = (runtime_config.advanced_features_status_bits >> bits_actuation_mode_global) & 1;

            break;

        case id_show_kb_reset:
            // Ничего
            break;

        default:
            // Ничего
            break;
    }
}

// Функция обмена данными между клавиатурой и VIA + сохранение данных в eeprom
void via_custom_value_command_kb(uint8_t *data, uint8_t lenght) {
    // data = [ command_id, channel_id, value_id, value_data ]
    uint8_t *command_id        = &(data[0]);
    uint8_t *channel_id        = &(data[1]);
    uint8_t *value_id_and_data = &(data[2]);

    if (*channel_id == id_custom_channel) {
        switch (*command_id) {
            case id_custom_set_value:
                via_custom_config_via_to_kb(value_id_and_data);

                break;

            case id_custom_get_value:
                via_custom_config_via_from_kb(value_id_and_data);

                break;

            case id_custom_save:
                break;

            default:
                // Ошибка по command_id
                *command_id = 77;
                break;
        }
        return;
    }
    // Ошибка по channel_id
    *command_id = 66;
}
