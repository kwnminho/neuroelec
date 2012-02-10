/*

  HPRGB2.cpp - Arduino library for High Power RGB LED Shield Version 2.0
  
*/


/* _____PROJECT INCLUDES_____________________________________________________ */
#include "HPRGB2.h"
#if defined(__PIC32MX__)
   #define PROGMEM
   #define pgm_read_word_near(x)	(*((short *)x))
   #define prog_uint16_t	const uint16_t
#else
   #include <avr/pgmspace.h>
#endif
/* _____PUBLIC FUNCTIONS_____________________________________________________ */
#define CIELPWM(a) (pgm_read_word_near(CIEL12 + a)) // CIE Lightness loopup table function

/*
12 bits PWM to CIE Luminance conversion
L* = 116(Y/Yn)^1/3 - 16 , Y/Yn > 0.008856
L* = 903.3(Y/Yn), Y/Yn <= 0.008856
*/
prog_uint16_t CIEL12[] PROGMEM = { 
        0,2,4,5,7,9,11,12,14,16,18,20,21,23,25,27,28,30,32,34,36,37,39,41,43,45,47,49,52,54,56,59,61,64,66,69,72,75,77,80,83,87,90,93,97,100,103,107,111,115,118,122,126,
        131,135,139,144,148,153,157,162,167,172,177,182,187,193,198,204,209,215,221,227,233,239,246,252,259,265,272,279,286,293,300,308,315,323,330,338,346,354,362,371,
        379,388,396,405,414,423,432,442,451,461,471,480,490,501,511,521,532,543,554,565,576,587,599,610,622,634,646,658,670,683,696,708,721,734,748,761,775,789,802,817,
        831,845,860,875,890,905,920,935,951,967,983,999,1015,1032,1048,1065,1082,1099,1117,1134,1152,1170,1188,1206,1225,1243,1262,1281,1301,1320,1340,1359,1379,1400,1420,
        1441,1461,1482,1504,1525,1547,1568,1590,1613,1635,1658,1681,1704,1727,1750,1774,1798,1822,1846,1871,1896,1921,1946,1971,1997,2023,2049,2075,2101,2128,2155,2182,2210,
        2237,2265,2293,2322,2350,2379,2408,2437,2467,2497,2527,2557,2587,2618,2649,2680,2712,2743,2775,2807,2840,2872,2905,2938,2972,3006,3039,3074,3108,3143,3178,3213,3248,
        3284,3320,3356,3393,3430,3467,3504,3542,3579,3617,3656,3694,3733,3773,3812,3852,3892,3932,3973,4013,4055,4095};
/*
Set address for MCP4728 and pca9685
*/

HPRGB::HPRGB(uint8_t mcp4728ID, uint8_t pca9685ID)
{
  _mcp4728ID = mcp4728ID;
  _mcp4728_address = (MCP4728_BASE_ADDR | _mcp4728ID);
  _pca9685ID = pca9685ID;
  _pca9685_address = (PCA9685_BASE_ADDR | _pca9685ID);
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
  pca9685Wake(); // wake PCA9685.
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
PCA9685 Color mixing commands
*/

/*
Default 8 bit input and 12 bit sRGB Gamma corrected PWM ouput
*/
void HPRGB::goToRGB(uint8_t red, uint8_t green, uint8_t blue)
{
  uint16_t values[3] = {CIELPWM(blue),CIELPWM(green),CIELPWM(red)};
  pca9685PWM(0,3,values);
}
/*
Default 8 bit input and 12 bit sRGB Gamma corrected PWM ouput
*/
void HPRGB::writeCH(uint8_t channel, uint8_t value)
{
  uint16_t values[1] = {CIELPWM(value)};
  pca9685PWM(channel,1,values);
}
/*
Direct 12bit PWM
*/
void HPRGB::writeCH12(uint8_t channel, uint16_t value)
{
  uint16_t values[1] = {value};
  pca9685PWM(channel,1,values);
}
/*
Direct 12bit PWM
*/
void HPRGB::goToRGB12(uint16_t red, uint16_t green, uint16_t blue)
{
  uint16_t values[3] = {blue,green,red};
  pca9685PWM(0,3,values);
}
/*
HSV 0..255
*/
void HPRGB::goToHSB(uint8_t hue, uint8_t saturation, uint8_t value)
{
  uint8_t r, g, b;
  HSBtoRGB(hue, saturation, value, &r, &g, &b);
  goToRGB(r, g, b);
}

/*
It is not really fade. Here for just compatiblibity with example code
*/
void HPRGB::fadeToHSB(uint8_t hue, uint8_t saturation, uint8_t value)
{
  uint8_t r, g, b;
  HSBtoRGB(hue, saturation, value, &r, &g, &b);
  goToRGB(r, g, b);
}
/*
Set PWM Frequency 40-1000Hz, Default 200Hz
*/
void HPRGB::setPWMFrequency(uint16_t freq)
{
  uint8_t prescale = round(25000000.000/4096.000/freq) - 1;
  pca9685PWMPreScale(prescale);
}

float HPRGB::getIntTemp()
{
  uint8_t hiTemp, lowTemp;
  Wire.beginTransmission(TMP421_ADDR);
  WIRE_WRITE((byte)0x00);
  Wire.endTransmission();
  Wire.requestFrom(TMP421_ADDR, 2); // request 2 byte from address 1001000
  while(Wire.available())
  {
  hiTemp = WIRE_READ(); // Read the first octet
  lowTemp = WIRE_READ(); // Read the second octet
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
  WIRE_WRITE(0x01);
  Wire.endTransmission();
  Wire.requestFrom(TMP421_ADDR, 2); // request 2 byte from address 1001000
  while(Wire.available())
  {
  hiTemp = WIRE_READ(); // Read the first octet
  lowTemp = WIRE_READ(); // Read the second octet
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
    WIRE_WRITE(highByte(_values[channel]));
    WIRE_WRITE(lowByte(_values[channel]));
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
    WIRE_WRITE(MULTIWRITE | (channel << 1)); 
    WIRE_WRITE(_intVref[channel] << 7 | _powerDown[channel] << 5 | _gain[channel] << 4 | highByte(_values[channel]));
    WIRE_WRITE(lowByte(_values[channel]));
  }
  return Wire.endTransmission();
}
/*
SingleWrite : send single register values to DAC EEPROM
update voltage referece, power down, gain, values on registers
*/
uint8_t HPRGB::mcp4728SingleWrite(uint8_t channel) {
  Wire.beginTransmission(_mcp4728_address);
  WIRE_WRITE(SINGLEWRITE | (channel << 1)); 
  WIRE_WRITE(_intVref[channel] << 7 | _powerDown[channel] << 5 | _gain[channel] << 4 | highByte(_values[channel]));
  WIRE_WRITE(lowByte(_values[channel]));
  return Wire.endTransmission();
}
/*
SequentialWrite : send four register values to DAC EEPROM
update voltage referece, power down, gain, values on registers
*/
uint8_t HPRGB::mcp4728SeqWrite() {
  Wire.beginTransmission(_mcp4728_address);
  WIRE_WRITE(SEQWRITE); 
  for (uint8_t channel=0; channel <= 3; channel++) {
    WIRE_WRITE(_intVref[channel] << 7 | _powerDown[channel] << 5 | _gain[channel] << 4 | highByte(_values[channel]));
    WIRE_WRITE(lowByte(_values[channel]));
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
    int deviceID = WIRE_READ();
    int hiByte = WIRE_READ();
    int loByte = WIRE_READ();

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
/*
Wake PCA9685 ociilator and enable auto increment
*/
void HPRGB::pca9685Wake()
{
  Wire.beginTransmission(_pca9685_address);
  WIRE_WRITE((byte)0x00);
  WIRE_WRITE(0B00100001);
  Wire.endTransmission();
}
/*
PCA9685 PWM frequency prescale
*/
void HPRGB::pca9685PWMPreScale(uint8_t prescale)
{
  Wire.beginTransmission(_pca9685_address);
  WIRE_WRITE(0xfe);
  WIRE_WRITE(prescale);
  Wire.endTransmission();
}

/*
Send PWM only OFF to channel, PWM start at 0. auto-incremental.
40% faster than single write and channel changes together.
*/
void HPRGB::pca9685PWM(uint8_t start, uint8_t numch, uint16_t * values)
{
  Wire.beginTransmission(_pca9685_address);
  byte startcmd = start*4+6;
  WIRE_WRITE((byte)startcmd);				// start from channel 0 ON
  for (int ch=0;ch<numch;ch++ )
  {
    WIRE_WRITE((byte)0x00);				// set all ON time to 0
    WIRE_WRITE((byte)0x00);
    WIRE_WRITE(lowByte(values[ch]));	// set OFF according to value
    WIRE_WRITE(highByte(values[ch]));
  }
  Wire.endTransmission();
}

uint16_t HPRGB::pca9685GetPWM(uint8_t channel) 
{
  channel = (channel * 4) + 6;
  Wire.beginTransmission(_pca9685_address);
  WIRE_WRITE(channel);
  Wire.endTransmission();
  Wire.requestFrom(_pca9685_address,(uint8_t)4);
  while(Wire.available()){
    uint8_t onLow = WIRE_READ();
    uint8_t onHi = WIRE_READ();
    uint8_t offLow = WIRE_READ();
    uint8_t offHi = WIRE_READ();
    return (word((offHi & 0B00001111), offLow) - word((onHi & 0B00001111), onLow));
  }
}
/*
HSV to RGB adopted from cyzRGB and adafruit
http://code.google.com/p/codalyze/source/browse/cyz_rgb/trunk/cyz/color.c
https://github.com/adafruit/RGB-matrix-Panel/blob/master/examples/plasma/plasma.pde
*/
void HPRGB::HSBtoRGB(uint8_t h, uint8_t s, uint8_t v, uint8_t* r, uint8_t* g, uint8_t* b)
{

        if ( s == 0 )
        {
                *b = *g = *r = v;
        }
        else
        {       
                
                uint8_t i = (h*6)/256;
                uint16_t f = (h*6) % 256;

                uint16_t p = (v * (255 - s)) / 256;
                uint16_t q = (v * (255 - (s * f)/256)) / 256;
                uint16_t t = (v * (255 - (s * (255 - f))/256)) / 256;

                if      ( i == 0 ) { *r = v ; *g = t ; *b = p; } // 0   deg (r)   to 60  deg (r+g)
                else if ( i == 1 ) { *r = q ; *g = v ; *b = p; } // 60  deg (r+g) to 120 deg (g)
                else if ( i == 2 ) { *r = p ; *g = v ; *b = t; } // 120 deg (g)   to 180 deg (g+b)
                else if ( i == 3 ) { *r = p ; *g = q ; *b = v; } // 180 deg (g+b) to 240 deg (b)
                else if ( i == 4 ) { *r = t ; *g = p ; *b = v; } // 240 deg (b)   to 300 deg (b+r)
                else               { *r = v ; *g = p ; *b = q; } // 300 deg (b+r) to 0   deg (r)
        }

}
