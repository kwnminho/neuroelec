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
mcp4728::mcp4728(uint8_t deviceID = 0x00)
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
  return _simpleCommand(RESET);
}
uint8_t mcp4728::wake() {
  return _simpleCommand(WAKE);
}
uint8_t mcp4728::update() {
  return _simpleCommand(UPDATE);
}

uint8_t mcp4728::_simpleCommand(byte simpleCommand) {
  Wire.beginTransmission(GENERALCALL);
  Wire.send(simpleCommand);
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
    Wire.send(MULTIWRITE | (channel << 1)); 
    Wire.send(_intVref[channel] << 7 | _powerDown[channel] << 5 | _gain[channel] << 4 | highByte(_values[channel]));
    Wire.send(lowByte(_values[channel]));
  }
  return Wire.endTransmission();
}

uint8_t mcp4728::analogWrite(uint16_t value1, uint16_t value2, uint16_t value3, uint16_t value4) {
  _values[0] = value1;
  _values[1] = value2;
  _values[2] = value3;
  _values[3] = value4;
  return fastWrite();
}

uint8_t mcp4728::analogWrite(uint8_t channel, uint16_t value) {
  _values[channel] = value;
  return fastWrite();
}

uint8_t mcp4728::singleWrite(uint8_t channel) {
  Wire.beginTransmission(_dev_address);
  Wire.send(SINGLEWRITE | (channel << 1)); 
  Wire.send(_intVref[channel] << 7 | _powerDown[channel] << 5 | _gain[channel] << 4 | highByte(_values[channel]));
  Wire.send(lowByte(_values[channel]));
  return Wire.endTransmission();
}

uint8_t mcp4728::seqWrite() {
  Wire.beginTransmission(_dev_address);
  Wire.send(SEQWRITE); 
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
  _valuesEp[0] = _values[0] = value1; 
  _valuesEp[1] = _values[1] = value2; 
  _valuesEp[2] = _values[2] = value3; 
  _valuesEp[3] = _values[3] = value4; 
  return seqWrite();
}

uint8_t mcp4728::eepromWrite()
{
  return seqWrite();
}

uint8_t mcp4728::writeVref() {
  Wire.beginTransmission(_dev_address);
  Wire.send(VREFWRITE | _intVref[0] << 3 | _intVref[1] << 2 | _intVref[2] << 1 | _intVref[3]); 
  return Wire.endTransmission();
}

uint8_t mcp4728::setVref(uint8_t value1, uint8_t value2, uint8_t value3, uint8_t value4) {
  _intVref[0] = value1;
  _intVref[1] = value2;
  _intVref[2] = value3;
  _intVref[3] = value4;
  return writeVref();
}
uint8_t mcp4728::setVref(uint8_t channel, uint8_t value) {
  _intVref[channel] = value;
  return writeVref();
}

uint8_t mcp4728::writeGain() {
  Wire.beginTransmission(_dev_address);
  Wire.send(GAINWRITE | _gain[0] << 3 | _gain[1] << 2 | _gain[2] << 1 | _gain[3]); 
  return Wire.endTransmission();
}

uint8_t mcp4728::setGain(uint8_t value1, uint8_t value2, uint8_t value3, uint8_t value4) {
  _gain[0] = value1;
  _gain[1] = value2;
  _gain[2] = value3;
  _gain[3] = value4;
  return writeGain();
}

uint8_t mcp4728::setGain(uint8_t channel, uint8_t value) {
  _gain[channel] = value;
  return writeGain();
}

uint8_t mcp4728::writePowerDown() {
  Wire.beginTransmission(_dev_address);
  Wire.send(POWERDOWNWRITE | _powerDown[0] << 2 | _powerDown[1]);
  Wire.send(_powerDown[2] << 6 | _powerDown[3] << 4); 
  return Wire.endTransmission();
}

uint8_t mcp4728::setPowerDown(uint8_t value1, uint8_t value2, uint8_t value3, uint8_t value4) {
  _powerDown[0] = value1;
  _powerDown[1] = value2;
  _powerDown[2] = value3;
  _powerDown[3] = value4;
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
uint8_t mcp4728::getVrefEp(uint8_t channel)
{
  return _intVrefEp[channel];
}
uint8_t mcp4728::getGainEp(uint8_t channel)
{
  return _gainEp[channel];
}
uint8_t mcp4728::getPowerDownEp(uint8_t channel)
{
  return _powerDownEp[channel];
}
uint16_t mcp4728::getValueEp(uint8_t channel)
{
  return _valuesEp[channel];
}
void mcp4728::vdd(uint16_t currentVdd)
{
  _vdd = currentVdd;
}

uint16_t mcp4728::getVout(uint8_t channel)
{
  uint32_t vref;
  if (_intVref[channel] == 1) {
      vref = 2048;
  }
  else {
      vref = _vdd;
  }

  uint32_t vOut = (vref * _values[channel] * (_gain[channel] + 1)) / 4096;
  if (vOut > _vdd) {
      vOut = _vdd;
  }
  return vOut;
}

void mcp4728::voutWrite(uint8_t channel, uint16_t vout)
{
  _vOut[channel] = vout;
  writeVout();
}

void mcp4728::voutWrite(uint16_t value1, uint16_t value2, uint16_t value3, uint16_t value4)
{
  _vOut[0] = value1;
  _vOut[1] = value2;
  _vOut[2] = value3;
  _vOut[3] = value4;
  writeVout();
}

void mcp4728::writeVout()
{
  for (uint8_t channel=0; channel <= 3; channel++) {
    if (_intVref[channel] == 1) {
      _values[channel] = _vOut[channel] / (_gain[channel] + 1) * 2;
    }
    else {
      _values[channel] = (long(_vOut[channel]) * 4096) / _vdd ;
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

