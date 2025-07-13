#include <Arduino.h>
#include "qrencode.h"
#include "qrcodeoled.h"

// Implements functions for OLED displays SSD1306 and SSH1106

QRcodeOled::QRcodeOled(OLEDDisplay *display) {
    
    this->display = display;
}

void QRcodeOled::init(int width, int height) {
    this->screenwidth = width;
    this->screenheight = height;
    display->init();
    //display->flipScreenVertically();
    display->setColor(WHITE);
    int min = screenwidth;
    if (screenheight<screenwidth)
        min = screenheight;
    multiply = min/WD;
    offsetsX = (screenwidth-(WD*multiply))/2;
    offsetsY = (screenheight-(WD*multiply))/2;
}

void QRcodeOled::init() {
    this->init(128,64);
}

void QRcodeOled::screenwhite() {
    display->clear();
    display->setColor(WHITE);
    display->fillRect(0, 0, screenwidth, screenheight);
    display->display();
}

void QRcodeOled::screenupdate() {
     display->display();
}

void QRcodeOled::drawPixel(int x, int y, int color) {
    OLEDDISPLAY_COLOR thecolor;
    if(color==1) {
        thecolor = BLACK;
    } else {
        thecolor = WHITE;
    }
    display->setColor(thecolor);
    display->setPixel(x, y);
    if (this->multiply>1) {
        display->setPixel(x+1,y);
        display->setPixel(x+1,y+1);
        display->setPixel(x,y+1);
    }
}
