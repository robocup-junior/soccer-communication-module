#ifndef FUNCTIONS_H
#define FUNCTIONS_H

String BLE_MAC_to_string();

int8_t module_set_indicator(String name);

String module_get_indicator();

void module_set_my_score(uint8_t score);

uint8_t module_get_my_score();

void module_set_opponent_score(uint8_t score);

uint8_t module_get_opponent_score();

void module_init_gpios();

void check_disconnect_button();

#endif // FUNCTIONS_H
