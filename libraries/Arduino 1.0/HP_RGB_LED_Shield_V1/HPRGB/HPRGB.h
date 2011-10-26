/**
Arduino library for High Power RGB LED Shield.
*/

#ifndef HPRGB_h
#define HPRGB_h

#include "Arduino.h"
#include <Wire.h>
#define senseR 0.14

#define MULTIWRITE 0B01000000
#define SINGLEWRITE 0B01011000
#define SEQWRITE 0B01010000
#define BASE_ADDR 0x60
#define TMP421_ADDR 0x4C

class HPRGB
{
  public:
    HPRGB(uint8_t mcp4728ID = 0x00, uint8_t cyzAddr = 0x09);
    void        begin();
    uint16_t    getCurrent(uint8_t);
    void        setCurrent(uint16_t, uint16_t, uint16_t);
    uint16_t    getFreq();
    void        setFreq(uint16_t);
    uint8_t     eepromWrite();
    uint16_t    getValue(uint8_t);
    uint8_t     goToRGB(uint8_t, uint8_t, uint8_t);
    uint8_t     fadeToRGB(uint8_t, uint8_t, uint8_t);
    uint8_t     fadeToHSB(uint8_t, uint8_t, uint8_t);
    uint8_t     fadeToRandomRGB(uint8_t, uint8_t, uint8_t);
    uint8_t     fadeToRandomHSB(uint8_t, uint8_t, uint8_t);
    uint8_t     playScript(uint8_t, uint8_t, uint8_t);
    uint8_t     stopScript();
    uint8_t     setFadeSpeed(uint8_t);
    uint8_t     setTimeAdj(uint8_t);
    uint8_t     setAddress(uint8_t);
    uint8_t     getAddress();
    uint8_t     setStartupParams(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
    void        getRGBColor(uint8_t*, uint8_t*, uint8_t*);
    uint8_t     setScriptLengthReps(uint8_t, uint8_t, uint8_t);
    float       getIntTemp();
    float       getExtTemp();
    float       getIntTempF();
    float       getExtTempF();
  private:
    void         getStatus();
    uint8_t      mcp4728FastWrite();
    uint8_t      mcp4728MultiWrite();
    uint8_t      mcp4728SingleWrite(uint8_t);
    uint8_t      mcp4728SeqWrite();
    uint8_t      _mcp4728_address;
    uint8_t      _mcp4728ID;
    uint8_t      _cyzAddr;	
    uint8_t      _intVref[4];
    uint8_t      _gain[4];
    uint8_t      _powerDown[4];
    uint16_t     _values[4];
    uint16_t     _valuesEp[4];
    uint8_t      _intVrefEp[4];
    uint8_t      _gainEp[4];
    uint8_t      _powerDownEp[4];
};
#endif

