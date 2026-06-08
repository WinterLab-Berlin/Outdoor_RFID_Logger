Firmware for a feeder with a RFID reader, SD card, GPS receiver and a pump.
GPS is used for precise time synchronization of RFID data. RFID date are stored on the SD card with time stamps.

The board in an [Adafruit Feather M0 Adalogger](https://www.adafruit.com/product/2796) with [Adafruit Ultimate GPS](https://www.adafruit.com/product/746).

# Install instructions

For an extensive overview see the [Adafruit instructions](https://learn.adafruit.com/adafruit-feather-m0-adalogger/setup) for the Arduino IDE version 1. Here is a short list of steps for Arduino IDE version 2.

1. Install the latest version of the Arduino IDE version 2. Install all requested drives and packages during the installation and in later steps
1. Open IDE
1. Go to the "Boards manager"
1. Install "Arduino SAMD Boards (32-bits ARM Cortex-M0+)" from Arduino
1. Go to Prefences under the File menu. In Settings copy and paste under "Additional boards manager URLs" the link: https://adafruit.github.io/arduino-board-index/package_adafruit_index.json For more details see [Adafruit instructions](https://learn.adafruit.com/adafruit-feather-m0-adalogger/setup)
1. Now you can install "Adafruit SAMD Boards" from Adafruit
1. Go to the "Library manager"
1. Install "Arduino Low Power" from Arduino with all dependancies
1. Install "RTCLib" from Arduino with all dependencies
1. Restart IDE
1. Move or unzip the "Outdoor_RFID_Logger" under "c:\Users\user_name\Documents\Arduino"
1. Open "Outdoor_RFID_Logger.ino" with Arduino IDE
1. Connect the device to PC
1. Under the boards selector click on "Select other board and port..."
1. In the "Search board" text box type M0 and select "Adafruit Feather M0 Express (SAMD21)". Select the serial port on the right side. Click on "OK"
1. Now you can program the device over the upload button
1. Copy the configuration file "conf.txt" to the SD card
1. SD card has to be formated with FAT32

# Troubleshooting

On fist compilation there can be an error from SD library. Just compile the project again and it should disappear.