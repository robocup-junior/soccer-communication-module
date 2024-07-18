#ifndef DISPLAY_H
#define DISPLAY_H

// Display library description https://github.com/ThingPulse/esp8266-oled-ssd1306

int8_t display_init();

int8_t display_screen_init();

int8_t display_screen_wait_for_connection();

int8_t display_screen_play();

int8_t display_screen_stop();

int8_t display_screen_damage(uint16_t time);

int8_t display_screen_half_break(uint16_t time);

int8_t display_screen_game_over();

#endif // DISPLAY_H
