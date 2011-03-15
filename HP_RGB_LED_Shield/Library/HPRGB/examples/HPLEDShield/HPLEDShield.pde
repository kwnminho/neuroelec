#include <Wire.h>
#include <HPRGB.h>


HPRGB ledShield; // default mcp4728 id(0) and default czyRGB address(9)

void setup()
{
  ledShield.begin();
  ledShield.setCurrent(350,350,350); // set maximum current for channel 1-3 (mA)
  ledShield.setFreq(600);// operation frequency of the LED driver (KHz)
  ledShield.eepromWrite();// write current settings to EEPROM
  delay(100); // wait for EEPROM writing
}


void loop()
{
  ledShield.goToRGB(255,255,255); delay (3000); //go to RGB color (white) now
  ledShield.fadeToRGB(255,0,0); delay (3000); //fade to RGB color (red)
  ledShield.fadeToRGB(0,255,0); delay (3000);//fade to RGB color (green)
  ledShield.fadeToRGB(0,0,255); delay (3000); //fade to RGB color (blue)
  ledShield.fadeToHSB(0,0,255); delay (3000); //fade to HSV color (Hue, Saturation, brightness)
}