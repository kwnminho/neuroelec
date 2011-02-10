#include <Wire.h>
#include "mcp4728.h"

mcp4728 dac = mcp4728(0); // instantiate mcp4728 object, Device ID = 0


void setup()
{
  Serial.begin(9600);  // initialize serial interface for print()
  dac.begin();  // initialize i2c interface
  dac.vdd(5.00); // set VDD of MCP4728 for correct conversion between LSB and Vout

/* If LDAC pin is not grounded, need to be pull down for normal operation.

  int LDACpin = 8;
  pinMode(LDACpin, OUTPUT); 
  digitalWrite(LDACpin, LOW);
*/
}


void loop()
{
/* 
  Debug print.
    Print all internal value and setting of input register and EEPROM. 
*/

  dac.print(); // serial print all current settings and values of input register and EEPROM.


/* 
  Basic analog writing to DAC
*/

  dac.analogWrite(500,500,500,500); // write to input register of DAC four channel (channel 0-3) together. Value 0-4095
  dac.analogWrite(0,700); // write to input register of a DAC. Channel 0-3, Value 0-4095
  int value = dac.getValue(0); // get current input register value of channel 0
  Serial.print("input register value of channel 0 = ");
  Serial.println(value, DEC); // serial print of value

  dac.voutWrite(1.8, 1.8, 1.8, 1.8) // write to input register of DAC. Value (V < VDD)
  dac.voutWrite(2, 1.6) // write to input register of DAC. Channel 0 - 3, Value (V < VDD)
  float vout = dac.getVout(1); // get current voltage out of channel 1
  Serial.print("Voltage out of channel 1 = "); 
  Serial.println(vout, DEC); // serial print of value


/* 
  Voltage reference settings
    Vref setting = 1 (internal), Gain = 0 (x1)   ==> Vref = 2.048V
    Vref setting = 1 (internal), Gain = 1 (x2)   ==> Vref = 4.096V
    Vref setting = 0 (external), Gain = ignored  ==> Vref = VDD
*/

  dac.setVref(1,1,1,1); // set to use internal voltage reference (2.048V)
  dac.setVref(0, 0); // set to use external voltage reference (=VDD, 2.7 - 5.5V)
  int vref = dac.getVref(1); // get current voltage reference setting of channel 1
  Serial.print("Voltage reference setting of channel 1 = "); // serial print of value
  Serial.println(vref, DEC); // serial print of value

  dac.setGain(0, 0, 0, 0); // set the gain of internal voltage reference ( 0 = gain x1, 1 = gain x2 )
  dac.setGain(1, 1); // set the gain of internal voltage reference ( 2.048V x 2 = 4.096V )
  int gain = dac.getGain(2); // get current gain setting of channel 2
  Serial.print("Gain setting of channel 2 = "); // serial print of value
  Serial.println(gain, DEC); // serial print of value


/*
  Power-Down settings
    0 = Normal , 1-3 = shut down most channel circuit, no voltage out and saving some power.
    1 = 1K ohms to GND, 2 = 100K ohms to GND, 3 = 500K ohms to GND
*/

  dac.setPowerDown(0, 0, 0, 0); // set Power-Down ( 0 = Normal , 1-3 = shut down most channel circuit, no voltage out) (1 = 1K ohms to GND, 2 = 100K ohms to GND, 3 = 500K ohms to GND)
  dac.setPowerDown(3, 1); // Power down channel 3 ( no voltage out from channel 3 )
  int powerDown = dac.getPowerDown(3); // get current Power-Down setting of channel 3
  Serial.print("PowerDown setting of channel 3 = "); // serial print of value
  Serial.println(powerDown, DEC); // serial print of value


/*
  Write values to EEPROM
    Writing value to EEPROM always update input register as well.
    Writing to EEPROM normally take about 50ms.
    To write Vref, Gain, Power-Down settings to EEPROM, just use eepromWrite() after set them in input registers.
*/
  dac.eepromWrite(1500,1500,1500,1500); // write to EEPROM of DAC four channel together. Value 0-4095
  delay(100);//writing to EEPROM takes about 50ms
  dac.eepromWrite(1, 1000); // write to EEPROM of DAC. Channel 0-3, Value 0-4095
  delay(100);//writing to EEPROM takes about 50ms
  dac.eepromWrite(); // write all input register values and settings to EEPROM
  dalay(100);//


/*
  Get Device ID (up to 8 devices can be used in a I2C bus, Device ID = 0-7)
*/

  int id = dac.getId(); // return devideID of object
  Serial.print("Device ID  = "); // serial print of value
  Serial.println(id, DEC); // serial print of value
}