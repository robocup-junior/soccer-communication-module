#ifndef ESPQRCODEOLED_H
#define ESPQRCODEOLED_H

/* ESP_QRcode. oled version
 * Import this .h when using some oled display
 */

#define OLEDDISPLAY

#include "qrcodedisplay.h"
#include "OLEDDisplay.h"

#define OLEDDISPLAY

class QRcodeOled : public QRcodeDisplay
{
	private:
        OLEDDisplay *display;
        void drawPixel(int x, int y, int color);
	public:
		QRcodeOled(OLEDDisplay *display);

		void init();
        void init(int width, int height);
		void screenwhite();
		void screenupdate();
};

#endif