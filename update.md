# How to update

### NO WARRANTY FOR DESTROYED HARDWARE
Be careful: USB ports provide 5 V. Incorrect wiring may damage your module or PC. Proceed only if you understand the risks. Use a USB breakout board if unsure.

## Requirements
- Communication module
- Arduino IDE(2)
- USB Cable that can be cut in half OR USB Breakout board
- Jumper cables female OR Jumper cables male with a breadboard

## Prepare your hardware
1. If using USB Cable: cut it in half and keep the half that can connects to your PC
2. If using female jumper cables: cut two cables in half and solder them to your usb cable OR your breakout board
3. Connect your module (Connect GND always first and disconnect last):
   1. USB GND (black) -> Module GND
   2. USB VCC (red) -> Module BAT+
   3. USB D+ (green) -> Module D+
   4. USB D- (white) -> Module D-
   
   NOTE:

    If your computer does not detect the device within ~10 seconds, swap D+ and D-.
   
    On some cables colors may differ - verify with a multimeter if possible.

4. (Connect your USB cable / breakout board with your PC)

## Prepare your PC
0. Install the Arduino IDE (2)
1. Open the Board manager `Tools > Board > Board Manager`
2. Search for `esp32`
3. Install `esp32 by Espressif Systems` in Version 3.2.2
4. Open the `*.ino` file
5. Connect your module
6. Under `Tools > Board > esp32` select `ESP32C6 Dev Module`
7. Under `Tools > Port` select the port where your COM Module is connected
8. Under `Sketch` select `Upload` to program the new firmware on your COM Module
9. DONE