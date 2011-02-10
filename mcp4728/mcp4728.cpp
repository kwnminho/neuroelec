/*

  mcp4728.cpp - Arduino library for MicroChip MCP4728 I2C D/A converter
  
*/


/* _____PROJECT INCLUDES_____________________________________________________ */
#include "mcp4728.h"


/* _____PUBLIC FUNCTIONS_____________________________________________________ */
/**
Constructor.

Creates class object, sets up the A/D storage array.
*/
mcp4728::mcp4728()
{
  _dev_address = BASE_ADDR;
  _vdd = defaultVDD;
}

mcp4728::mcp4728(uint8_t deviceID)
{
  _deviceID = deviceID;
  _dev_address = (BASE_ADDR | _deviceID);
  _vdd = defaultVDD;
}

void mcp4728::begin()
{
  Wire.begin();
  getStatus();
}
uint8_t mcp4728::reset() {
  Wire.beginTransmission(0B0000000);
  Wire.send(0B00000110);
  return Wire.endTransmission();
}
uint8_t mcp4728::wake() {
  Wire.beginTransmission(0B0000000);
  Wire.send(0B00001001);
  return Wire.endTransmission();
}
uint8_t mcp4728::update() {
  Wire.beginTransmission(0B0000000);
  Wire.send(0B00001000);
  return Wire.endTransmission();
}
uint8_t mcp4728::fastWrite() {
  Wire.beginTransmission(_dev_address);
  for (uint8_t channel=0; channel <= 3; channel++) {
    Wire.send(highByte(_values[channel]));
    Wire.send(lowByte(_values[channel]));
  }
  return Wire.endTransmission();
}

uint8_t mcp4728::multiWrite() {
  Wire.beginTransmission(_dev_address);
  for (uint8_t channel=0; channel <= 3; channel++) {
    Wire.send(0B01000000 | (channel << 1)); 
    Wire.send(_intVref[channel] << 7 | _powerDown[channel] << 5 | _gain[channel] << 4 | highByte(_values[channel]));
    Wire.send(lowByte(_values[channel]));
  }
  return Wire.endTransmission();
}

uint8_t mcp4728::analogWrite(uint16_t value1, uint16_t value2, uint16_t value3, uint16_t value4) {
  uint16_t values[4] = {value1, value2, value3, value4};
  for (uint8_t channel=0; channel <= 3; channel++) {
    _values[channel] = values[channel];
  }
  return fastWrite();
}

uint8_t mcp4728::analogWrite(uint8_t channel, uint16_t value) {
  _values[channel] = value;
  return fastWrite();
}

uint8_t mcp4728::singleWrite(uint8_t channel) {
  Wire.beginTransmission(_dev_address);
  Wire.send(0B01011000 | (channel << 1)); 
  Wire.send(_intVref[channel] << 7 | _powerDown[channel] << 5 | _gain[channel] << 4 | highByte(_values[channel]));
  Wire.send(lowByte(_values[channel]));
  return Wire.endTransmission();
}

uint8_t mcp4728::seqWrite() {
  Wire.beginTransmission(_dev_address);
  Wire.send(0B01010000); 
  for (uint8_t channel=0; channel <= 3; channel++) {
    Wire.send(_intVref[channel] << 7 | _powerDown[channel] << 5 | _gain[channel] << 4 | highByte(_values[channel]));
    Wire.send(lowByte(_values[channel]));
  }
  return Wire.endTransmission();
}

uint8_t mcp4728::eepromWrite(uint8_t channel, uint16_t value)
{
  _values[channel] = value;
  _valuesEp[channel] = value;
  return singleWrite(channel);
}

uint8_t mcp4728::eepromWrite(uint16_t value1, uint16_t value2, uint16_t value3, uint16_t value4)
{
  uint16_t values[4] = {value1, value2, value3, value4};
  for (uint8_t channel=0; channel <= 3; channel++) {
    _values[channel] = values[channel];
    _valuesEp[channel] = values[channel];
  }
  return seqWrite();
}

uint8_t mcp4728::eepromWrite()
{
  return seqWrite();
}

uint8_t mcp4728::writeVref() {
  Wire.beginTransmission(_dev_address);
  Wire.send(0B10000000 | _intVref[0] << 3 | _intVref[1] << 2 | _intVref[2] << 1 | _intVref[3]); 
  return Wire.endTransmission();
}

uint8_t mcp4728::setVref(uint8_t value1, uint8_t value2, uint8_t value3, uint8_t value4) {
  uint8_t values[4] = {value1, value2, value3, value4};
  for (uint8_t channel=0; channel <= 3; channel++) {
    _intVref[channel] = values[channel];
  }
  return writeVref();
}
uint8_t mcp4728::setVref(uint8_t channel, uint8_t value) {
  _intVref[channel] = value;
  return writeVref();
}

uint8_t mcp4728::writeGain() {
  Wire.beginTransmission(_dev_address);
  Wire.send(0B11000000 | _gain[0] << 3 | _gain[1] << 2 | _gain[2] << 1 | _gain[3]); 
  return Wire.endTransmission();
}

uint8_t mcp4728::setGain(uint8_t value1, uint8_t value2, uint8_t value3, uint8_t value4) {
  uint8_t values[4] = {value1, value2, value3, value4};
  for (uint8_t channel=0; channel <= 3; channel++) {
    _gain[channel] = values[channel];
  }
  return writeGain();
}

uint8_t mcp4728::setGain(uint8_t channel, uint8_t value) {
  _gain[channel] = value;
  return writeGain();
}

uint8_t mcp4728::writePowerDown() {
  Wire.beginTransmission(_dev_address);
  Wire.send(0B10100000 | _powerDown[0] << 2 | _powerDown[1]);
  Wire.send(_powerDown[2] << 6 | _powerDown[3] << 4); 
  return Wire.endTransmission();
}

uint8_t mcp4728::setPowerDown(uint8_t value1, uint8_t value2, uint8_t value3, uint8_t value4) {
  uint8_t values[4] = {value1, value2, value3, value4};
  for (uint8_t channel=0; channel <= 3; channel++) {
    _powerDown[channel] = values[channel];
  }
  return writePowerDown();
}
uint8_t mcp4728::setPowerDown(uint8_t channel, uint8_t value) {
  _powerDown[channel] = value;
  return writePowerDown();
}

uint8_t mcp4728::getId()
{
  return _deviceID;
}

uint8_t mcp4728::getVref(uint8_t channel)
{
  return _intVref[channel];
}
uint8_t mcp4728::getGain(uint8_t channel)
{
  return _gain[channel];
}
uint8_t mcp4728::getPowerDown(uint8_t channel)
{
  return _powerDown[channel];
}
uint16_t mcp4728::getValue(uint8_t channel)
{
  return _values[channel];
}
void mcp4728::vdd(float currentVdd)
{
  _vdd = currentVdd;
}

float mcp4728::getVout(uint8_t channel)
{
  float vref;
  if (_intVref[channel] == 1) {
      vref = 2.048;
  }
  else {
      vref = _vdd;
  }
  float vOut = vref * (_gain[channel] + 1) * _values[channel] / 4096;
  if (vOut > _vdd) {
      vOut = _vdd;
  }
  return vOut;
}

void mcp4728::voutWrite(uint8_t channel, float vout)
{
  _vOut[channel] = vout;
  writeVout();
}

void mcp4728::voutWrite(float value1, float value2, float value3, float value4)
{
  float vref;
  float values[4] = {value1, value2, value3, value4};
  for (uint8_t channel=0; channel <= 3; channel++) {
    _vOut[channel] = values [channel];
  }
  writeVout();
}

void mcp4728::writeVout()
{
  float vref;
  for (uint8_t channel=0; channel <= 3; channel++) {
    if (_intVref[channel] == 1) {
      _values[channel] = _vOut[channel] / 2.048 / (_gain[channel] + 1) * 4096;
    }
    else {
      _values[channel] = _vOut[channel] / _vdd * 4096;
    }
  }
  fastWrite();
}

void mcp4728::getStatus()
{
  Wire.requestFrom(int(_dev_address), 24);
  while(Wire.available())
  { 
    int deviceID = Wire.receive();
    int hiByte = Wire.receive();
    int loByte = Wire.receive();

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


void mcp4728::print()
{
  Serial.println("NAME     Vref  Gain  PowerDown  Value");
  for (uint8_t channel=0; channel <= 3; channel++)
  { 
    Serial.print("DAC");
    Serial.print(channel,DEC);
    Serial.print("   ");
    Serial.print("    "); 
    Serial.print(_intVref[channel],BIN);
    Serial.print("     ");
    Serial.print(_gain[channel],BIN);
    Serial.print("       ");
    Serial.print(_powerDown[channel],BIN);
    Serial.print("       ");
    Serial.println(_values[channel],DEC);

    Serial.print("EEPROM");
    Serial.print(channel,DEC);
    Serial.print("    "); 
    Serial.print(_intVrefEp[channel],BIN);
    Serial.print("     ");
    Serial.print(_gainEp[channel],BIN);
    Serial.print("       ");
    Serial.print(_powerDownEp[channel],BIN);
    Serial.print("       ");
    Serial.println(_valuesEp[channel],DEC);
  }
  Serial.println(" ");
}

float mcp4728::getCurrent(uint8_t channel) {
  float currentSet;
  float ctrlV = getVout(channel);
  if (ctrlV >= 1) {
    currentSet = 100/senseR;
  }
  else {
    currentSet = ctrlV/senseR/10;
  }
  return (currentSet * 1000);
}

void mcp4728::setCurrent(uint8_t channel, uint16_t currentSet) {
  float ctrlV = float(currentSet) / 1000 * senseR * 10;
  _vOut[channel+1] = ctrlV;
  writeVout();
}

void mcp4728::setCurrent(uint16_t currentSet1, uint16_t currentSet2, uint16_t currentSet3) {
  float ctrlV;
  float values[3] = {currentSet1, currentSet2, currentSet3};
  for (uint8_t channel=0; channel <= 2; channel++) {
    ctrlV = values[channel] / 1000 * senseR * 10;
    _vOut[channel+1] = ctrlV;
  }
  writeVout();
}

float mcp4728::getFreq() {
  float currentFreq;
  float fAdjV = getVout(0);
  if (fAdjV >= 1) {
    currentFreq = 2100;
  }
  else {
    if (fAdjV > 0.2) {
      currentFreq = (-727.902) * sq(fAdjV) + 2751.735 * fAdjV + 54.461;
    }
    else {
      currentFreq = 2691.5 * fAdjV + 44.383;
    }
  }
  return currentFreq;
}

void mcp4728::setFreq(uint16_t freq) {
  float fAdjV;
  float freqSet = float(freq);
  if (freqSet < 600){
    fAdjV =  0.000000069948 * sq(freqSet) + 0.00030705 * freqSet + 0.00225637;
  }
  else{
    fAdjV =  0.000371434 * freqSet - 0.016439;
  }
  _vOut[0] = fAdjV;
  writeVout();
}
