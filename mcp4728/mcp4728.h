/**
Arduino library for MicroChip MCP4728 I2C D/A converter.
*/

#ifndef mcp4728_h
#define mcp4728_h

#include "WProgram.h"
#include <Wire.h>

#define senseR 0.13
#define defaultVDD 5

class mcp4728
{
  public:
    mcp4728();
    mcp4728(uint8_t);
    void     vdd(float);
    void     begin();
    uint8_t  reset();
    uint8_t  wake();
    uint8_t  update();
    uint8_t  analogWrite(uint16_t, uint16_t, uint16_t, uint16_t);
    uint8_t  analogWrite(uint8_t, uint16_t);
    uint8_t  eepromWrite(uint16_t, uint16_t, uint16_t, uint16_t);
    uint8_t  eepromWrite(uint8_t, uint16_t);
    uint8_t  eepromWrite();
    void     print();
    uint8_t  setVref(uint8_t, uint8_t, uint8_t, uint8_t);
    uint8_t  setVref(uint8_t, uint8_t);
    uint8_t  setGain(uint8_t, uint8_t, uint8_t, uint8_t);
    uint8_t  setGain(uint8_t, uint8_t);
    uint8_t  setPowerDown(uint8_t, uint8_t, uint8_t, uint8_t);
    uint8_t  setPowerDown(uint8_t, uint8_t);
    uint8_t  getId();
    uint8_t  getVref(uint8_t);
    uint8_t  getGain(uint8_t);
    uint8_t  getPowerDown(uint8_t);
    uint16_t getValue(uint8_t);
    float    getVout(uint8_t);
    void     voutWrite(uint8_t, float);
    void     voutWrite(float, float, float, float);
    float    getCurrent(uint8_t);
    void     setCurrent(uint8_t, uint16_t);
    void     setCurrent(uint16_t, uint16_t, uint16_t);
    float    getFreq();
    void     setFreq(uint16_t);
    void     readAddress();
    void     writeAddress(uint8_t, uint8_t);
  private:
    void         getStatus();
    uint8_t      fastWrite();
    uint8_t      multiWrite();
    uint8_t      singleWrite(uint8_t);
    uint8_t      seqWrite();
    uint8_t      writeVref();
    uint8_t      writeGain();
    uint8_t      writePowerDown();
    void         writeVout();
    static const uint8_t BASE_ADDR                 = 0x60;
    uint8_t      _dev_address;
    uint8_t      _deviceID;
    uint8_t      _intVref[4];
    uint8_t      _gain[4];
    uint8_t      _powerDown[4];
    uint16_t     _values[4];
    uint16_t     _valuesEp[4];
    uint8_t      _intVrefEp[4];
    uint8_t      _gainEp[4];
    uint8_t      _powerDownEp[4];
    float        _vOut[4];
    float        _vdd;
};
#endif

