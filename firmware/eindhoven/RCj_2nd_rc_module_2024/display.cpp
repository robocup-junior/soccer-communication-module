#include "WString.h"
#include "OLEDDisplay.h"
#include "esp_err.h"
#include <SSD1306.h>
#include <sys/_stdint.h>
#include "qrcodeoled.h"

#include "definitions.h"
#include "images.h"
#include "functions.h"
#include "fonts.h"
#include "display.h"

static SSD1306  display(0x3c, I2C_SDA_GPIO, I2C_SCL_GPIO);
QRcodeOled qrcode (&display);


// Static functions
static String get_time_string(uint16_t seconds) {
    if (seconds < 60) {
        return String(seconds);
    }
    return String(seconds / 60) + ":" + ((seconds % 60 < 10) ? "0" : "") + String(seconds % 60);
}

static String get_score_string() {
    return String(module_get_my_score()) + ":" + String(module_get_opponent_score());
}


// Public functions
int8_t display_init() {
    display.init();
    return ESP_OK;
}


// Screens
int8_t display_screen_init() {
    display.clear();
    display.drawXbm(0, 0, 128, 64, RC_logo);

    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(128, 54, ("v " + String(FW_VERSION_MAJOR) + "." + String(FW_VERSION_MINOR)));

    display.display();
    return ESP_OK;
}

int8_t display_screen_wait_for_connection() { 
    String mac_string = BLE_MAC_to_string();

    display.clear();

    qrcode.init(64, 64);
    qrcode.create(mac_string);

    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);

    display.drawString(96, 2, "Wait for\nconnection");
    display.drawString(96, 39, (mac_string.substring(0,9) + "\n" + mac_string.substring(9)));

    display.display();
    return ESP_OK;
}

int8_t display_screen_play() { 
    display.clear();

    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 0, "PLAY");


    display.setFont(DialogInput_bold_50);   
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(41, 2, module_get_indicator().substring(0,1));
    display.drawString(85, 2, module_get_indicator().substring(1,2));

    display.setFont(Dialog_bold_12);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 52, get_score_string());

    display.display();
    return ESP_OK;
}

int8_t display_screen_stop() { 
    display.clear();

    display.fillRect(0, 0, 18, 64);
    display.fillRect(110, 0, 128, 64);
    

    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 0, "STOP");


    display.setFont(DialogInput_bold_50);   
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(41, 2, module_get_indicator().substring(0,1));
    display.drawString(85, 2, module_get_indicator().substring(1,2));

    display.setFont(Dialog_bold_12);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 52, get_score_string());

    //display.invertDisplay();
    display.display();
    return ESP_OK;
}

int8_t display_screen_damage(uint16_t time) { 
    display.clear();
    

    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 0, "PENALTY - " + module_get_indicator().substring(0,2));


    display.setFont(Dialog_plain_40);   
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 10, get_time_string(time));

    display.setFont(Dialog_bold_12);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 52, get_score_string());

    //display.invertDisplay();
    display.display();
    return ESP_OK;
}

int8_t display_screen_half_break(uint16_t time) { 
    display.clear();
    

    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 0, "HALFTIME - " + module_get_indicator().substring(0,2));


    display.setFont(Dialog_plain_40);   
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 10, get_time_string(time));

    display.setFont(Dialog_bold_12);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 52, get_score_string());

    //display.invertDisplay();
    display.display();
    return ESP_OK;
}

int8_t display_screen_game_over() { 
    display.clear();

    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 0, "GAME OVER");


    display.setFont(Dialog_plain_40);   
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 10, get_score_string());

    display.setFont(Dialog_bold_12);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 52, module_get_indicator().substring(0,2));

    //display.invertDisplay();
    display.display();
    return ESP_OK;
}
