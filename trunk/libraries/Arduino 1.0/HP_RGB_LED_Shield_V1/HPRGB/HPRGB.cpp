/*

  HPRGB.cpp - Arduino library for High Power RGB LED Shield
  
*/


/* _____PROJECT INCLUDES_____________________________________________________ */
#include "HPRGB.h"

/* _____PUBLIC FUNCTIONS_____________________________________________________ */

/*
Set address for MCP4728 and attiny85
*/

HPRGB::HPRGB(uint8_t mcp4728ID, uint8_t cyzAddr)
{
  _mcp4728ID = mcp4728ID;
  _mcp4728_address = (BASE_ADDR | _mcp4728ID);
  _cyzAddr = cyzAddr;
}

/*
Wire.begin for I2C.
Get current settings from MCP4728. Set proper settings (voltage reference, gain, powerdown mode in case)
*/

void HPRGB::begin()
{
  Wire.begin();
  getStatus(); // read the values from mcp4728
  for (uint8_t ch=0;ch>=4 ;ch++ )
  {
    _intVref[ch] = 1; // set all channels to use internal voltage reference (2.048V)
	_gain[ch] = 0; // Gain = x1
	_powerDown[ch] = 0; // Regaulr running mode
  }
  mcp4728FastWrite(); // write initial values to input registers.
}

/*
Write current values and settings to MCP4728 EEPROM
*/

uint8_t HPRGB::eepromWrite()
{
  return mcp4728SeqWrite();
}

/*
Return current value for a channel (value / 4098)
*/

uint16_t HPRGB::getValue(uint8_t channel)
{
  return _values[channel];
}
/*
Return maximum current setting of the channel (mA)
*/

uint16_t HPRGB::getCurrent(uint8_t channel) {
  uint16_t currentSet;
  uint16_t ctrlV = _values[channel];
  if (ctrlV >= 2000) {
    currentSet = 100/senseR;
  }
  else {
    currentSet = ctrlV/senseR/20;
  }
  return (currentSet);
}
/*
Set maximum current setting of the channels (mA)
*/
void HPRGB::setCurrent(uint16_t currentSet1, uint16_t currentSet2, uint16_t currentSet3) {
  uint16_t values[3] = {currentSet1, currentSet2, currentSet3};
  for (uint8_t channel=0; channel <= 2; channel++) {
    _values[channel+1] = values[channel] * senseR * 20;
  }
  mcp4728FastWrite();
}
/*
Return operating frequency setting (KHz)
Due to the non-linear curve of freq, +-1% inaccuracy
*/

uint16_t HPRGB::getFreq() {
  uint16_t currentFreq;
  uint16_t fAdjV = _values[0];
  if (fAdjV >= 2000) {
    currentFreq = 2100;
  }
  else {
    currentFreq = ((-0.000173694) * fAdjV + 1.36185 ) * fAdjV + 62.5883; // second order polynominal fitting to the curve (freq vs DAC steps)
  }
  return currentFreq;
}
/*
set operating frequency (KHz)
Due to the non-linear curve of freq, +-1% inaccuracy
*/
void HPRGB::setFreq(uint16_t freq) {
  _values[0] =  0.0001787 * freq * freq + 0.56895 * freq + 8.0598; // second order polynominal fitting to the curve (freq vs DAC steps)
  mcp4728FastWrite();
}

/*
CyzRGB commands for PWM dimming and RGB color mixing
Commands are based on CyzRGB wiki and blinkM datasheet.
Only partially implemented
*/

uint8_t HPRGB::goToRGB(uint8_t red, uint8_t green, uint8_t blue)
{
  Wire.beginTransmission(_cyzAddr);
  Wire.write('n');
  Wire.write(red);
  Wire.write(green);
  Wire.write(blue);
  return Wire.endTransmission();
}
uint8_t HPRGB::fadeToRGB(uint8_t red, uint8_t green, uint8_t blue)
{
  Wire.beginTransmission(_cyzAddr);
  Wire.write('c');
  Wire.write(red);
  Wire.write(green);
  Wire.write(blue);
  return Wire.endTransmission();
}

uint8_t HPRGB::fadeToHSB(uint8_t hue, uint8_t saturation, uint8_t brightness)
{
  Wire.beginTransmission(_cyzAddr);
  Wire.write('h');
  Wire.write(hue);
  Wire.write(saturation);
  Wire.write(brightness);
  return Wire.endTransmission();
}

uint8_t HPRGB::fadeToRandomRGB(uint8_t red, uint8_t green, uint8_t blue)
{
  Wire.beginTransmission(_cyzAddr);
  Wire.write('C');
  Wire.write(red);
  Wire.write(green);
  Wire.write(blue);
  return Wire.endTransmission();
}

uint8_t HPRGB::fadeToRandomHSB(uint8_t hue, uint8_t saturation, uint8_t brightness)
{
  Wire.beginTransmission(_cyzAddr);
  Wire.write('H');
  Wire.write(hue);
  Wire.write(saturation);
  Wire.write(brightness);
  return Wire.endTransmission();
}

uint8_t HPRGB::playScript(uint8_t scriptNumber, uint8_t repeat, uint8_t position)
{
  Wire.beginTransmission(_cyzAddr);
  Wire.write('p');
  Wire.write(scriptNumber);
  Wire.write(repeat);
  Wire.write(position);
  return Wire.endTransmission();
}

uint8_t HPRGB::stopScript()
{
  Wire.beginTransmission(_cyzAddr);
  Wire.write('o');
  return Wire.endTransmission();
}

uint8_t HPRGB::setFadeSpeed(uint8_t fadespeed)
{
  Wire.beginTransmission(_cyzAddr);
  Wire.write('f');
  Wire.write(fadespeed);
  return Wire.endTransmission();  
}

uint8_t HPRGB::setTimeAdj(uint8_t timeadj)
{
  Wire.beginTransmission(_cyzAddr);
  Wire.write('t');
  Wire.write(timeadj);
  return Wire.endTransmission();  
}

void HPRGB::getRGBColor(uint8_t* red, uint8_t* green, uint8_t* blue)
{
  Wire.beginTransmission(_cyzAddr);
  Wire.write('g');
  Wire.endTransmission();
  Wire.requestFrom(_cyzAddr,(uint8_t)3);
  if( Wire.available() ) {
    *red = Wire.read();
    *green = Wire.read();
    *blue = Wire.read();
  }
}

uint8_t HPRGB::setScriptLengthReps(uint8_t scriptNumber, uint8_t length, uint8_t repeat)
{
  Wire.beginTransmission(_cyzAddr);
  Wire.write('L');
  Wire.write(scriptNumber);
  Wire.write(length);
  Wire.write(repeat);
  return Wire.endTransmission();
}

uint8_t HPRGB::setAddress(uint8_t addr)
{
  Wire.beginTransmission(0x00);
  Wire.write('A');
  Wire.write(addr);
  Wire.write((byte)0xD0);
  Wire.write((byte)0x0D);
  Wire.write(addr);
  return Wire.endTransmission();
  delay(50);
}

uint8_t HPRGB::getAddress()
{
  Wire.beginTransmission(_cyzAddr);
  Wire.write('a');
  Wire.endTransmission();
  Wire.requestFrom(_cyzAddr, (uint8_t)1);
  if( Wire.available() ) {
    uint8_t addr = Wire.read();
    return addr;
  }
  return -1;
}

uint8_t HPRGB::setStartupParams(uint8_t mode, uint8_t scriptNumber, uint8_t repeat, uint8_t fadespeed, uint8_t timeadj)
{
  Wire.beginTransmission(_cyzAddr);
  Wire.write('B');
  Wire.write(mode);
  Wire.write(scriptNumber);
  Wire.write(repeat);
  Wire.write(fadespeed);
  Wire.write(timeadj);
  return Wire.endTransmission();
} 

float HPRGB::getIntTemp()
{
  uint8_t hiTemp, lowTemp;
  Wire.beginTransmission(TMP421_ADDR);
  Wire.write((byte)0x00);
  Wire.endTransmission();
  Wire.requestFrom(TMP421_ADDR, 2); // request 2 byte from address 1001000
  while(Wire.available())
  {
  hiTemp = Wire.read(); // Read the first octet
  lowTemp = Wire.read(); // Read the second octet
  }
  return (hiTemp + (lowTemp >> 4)*0.0625);
}
float HPRGB::getIntTempF()
{
  return (getIntTemp()*9/5 + 32);
}

float HPRGB::getExtTemp()
{
  uint8_t hiTemp, lowTemp;
  Wire.beginTransmission(TMP421_ADDR);
  Wire.write((byte)0x01);
  Wire.endTransmission();
  Wire.requestFrom(TMP421_ADDR, 2); // request 2 byte from address 1001000
  while(Wire.available())
  {
  hiTemp = Wire.read(); // Read the first octet
  lowTemp = Wire.read(); // Read the second octet
  }
  return (hiTemp + (lowTemp >> 4)*0.0625);
} 
float HPRGB::getExtTempF()
{
  return (getExtTemp()*9/5 + 32);
}

/* _____PRIVATE FUNCTIONS_____________________________________________________ */

/*
FastWrite : send only input register values to DAC
No command byte.
*/

uint8_t HPRGB::mcp4728FastWrite() {
  Wire.beginTransmission(_mcp4728_address);
  for (uint8_t channel=0; channel <= 3; channel++) {
    Wire.write(highByte(_values[channel]));
    Wire.write(lowByte(_values[channel]));
  }
  return Wire.endTransmission();
}
/*
MultiWrite : send all register values to DAC
update voltage referece, power down, gain, values on registers
*/
uint8_t HPRGB::mcp4728MultiWrite() {
  Wire.beginTransmission(_mcp4728_address);
  for (uint8_t channel=0; channel <= 3; channel++) {
    Wire.write(MULTIWRITE | (channel << 1)); 
    Wire.write(_intVref[channel] << 7 | _powerDown[channel] << 5 | _gain[channel] << 4 | highByte(_values[channel]));
    Wire.write(lowByte(_values[channel]));
  }
  return Wire.endTransmission();
}
/*
SingleWrite : send single register values to DAC EEPROM
update voltage referece, power down, gain, values on registers
*/
uint8_t HPRGB::mcp4728SingleWrite(uint8_t channel) {
  Wire.beginTransmission(_mcp4728_address);
  Wire.write(SINGLEWRITE | (channel << 1)); 
  Wire.write(_intVref[channel] << 7 | _powerDown[channel] << 5 | _gain[channel] << 4 | highByte(_values[channel]));
  Wire.write(lowByte(_values[channel]));
  return Wire.endTransmission();
}
/*
SequentialWrite : send four register values to DAC EEPROM
update voltage referece, power down, gain, values on registers
*/
uint8_t HPRGB::mcp4728SeqWrite() {
  Wire.beginTransmission(_mcp4728_address);
  Wire.write(SEQWRITE); 
  for (uint8_t channel=0; channel <= 3; channel++) {
    Wire.write(_intVref[channel] << 7 | _powerDown[channel] << 5 | _gain[channel] << 4 | highByte(_values[channel]));
    Wire.write(lowByte(_values[channel]));
  }
  return Wire.endTransmission();
}
/*
Get current values and settings from MCP4728 for initialization
*/
void HPRGB::getStatus()
{
  Wire.requestFrom(int(_mcp4728_address), 24);
  while(Wire.available())
  { 
    int deviceID = Wire.read();
    int hiByte = Wire.read();
    int loByte = Wire.read();

    int isEEPROM = (deviceID & 0B00001000) >> 3;
    int channel = (deviceID & 0B00110000) >> 4;
    if (isEEPROM == 1) {
      _intVrefEp[channel] = (hiByte & 0B10000000) >> 7;
      _gainEp[channel] = (hiByte & 0B00010000) >> 4;
      _powerDownEp[channel] = (hiByte & 0B01100000) >> 5;
      _valuesEp[channel] = word((hiByte & 0B00001111), loByte);
    }
    else {
      _intVref[channel] = (hiByte & 0B10000000) >> 7;
      _gain[channel] = (hiByte & 0B00010000) >> 4;
      _powerDown[channel] = (hiByte & 0B01100000) >> 5;
      _values[channel] = word((hiByte & 0B00001111), loByte);
    }
  }
}