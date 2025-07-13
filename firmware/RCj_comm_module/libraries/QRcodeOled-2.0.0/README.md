# QRcodeOled

Subclass that you can use jointly with https://github.com/yoprogramo/QRcodeDisplay to generate QRcodes in OLED displays like SSD1306 and SSH1106

To use it:

## In platform.io: 

add as dependencies:

```
yoprogramo/QRcodeDisplay
yoprogramo/QRcodeOled
thingpulse/ESP8266 and ESP32 OLED driver for SSD1306 displays
```

## In arduino ide:
open Library Manager (menu Sketch > Include Library > Manage Libraries…) then install the following libraries:

```
 QRcodeDisplay
 QRcodeOled
 ESP8266 and ESP32 OLED driver for SSD1306 displays
```

 Creating a QRcode is just as simple as:

 ```
#include <qrcodeoled.h>
#include <SSD1306.h>

SSD1306  display(0x3c, 21, 22); // Only change
QRcodeOled qrcode (&display);

void setup() {

    qrcode.init();
    qrcode.create("Hello world.");

}

 ```

 For other displays, please refer the main repository: https://github.com/yoprogramo/QRcodeDisplay
