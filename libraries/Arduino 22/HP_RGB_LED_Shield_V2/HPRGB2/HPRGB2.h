/**
Arduino library for High Power RGB LED Shield.
*/

#ifndef HPRGB2_h
#define HPRGB2_h

#include "WProgram.h"
#include <Wire.h>
#define senseR 0.14

#define MULTIWRITE 0B01000000
#define SINGLEWRITE 0B01011000
#define SEQWRITE 0B01010000
#define TMP421_ADDR 0x4C

class HPRGB
{
  public:
    HPRGB(uint8_t mcp4728ID = 0x00, uint8_t pca9685ID = 0x00);
    void        begin();
    uint16_t    getCurrent(uint8_t);
    void        setCurrent(uint16_t, uint16_t, uint16_t);
    uint16_t    getFreq();
    void        setFreq(uint16_t);
    uint8_t     eepromWrite();
    uint16_t    getValue(uint8_t);
    void        goToRGB(uint8_t, uint8_t, uint8_t);
    void        goToRGB12(uint16_t, uint16_t, uint16_t);
    void        goToHSB(uint8_t, uint8_t, uint8_t);
    void        fadeToHSB(uint8_t, uint8_t, uint8_t);
    void        setPWMFrequency(uint16_t frequency);
    float       getIntTemp();
    float       getExtTemp();
    float       getIntTempF();
    float       getExtTempF();

  private:
    static const uint8_t MCP4728_BASE_ADDR                 = 0x60; // 96 - 104
    void         getStatus();
    uint8_t      mcp4728FastWrite();
    uint8_t      mcp4728MultiWrite();
    uint8_t      mcp4728SingleWrite(uint8_t);
    uint8_t      mcp4728SeqWrite();
    uint8_t      _mcp4728_address;
    uint8_t      _mcp4728ID;
    uint8_t      _intVref[4];
    uint8_t      _gain[4];
    uint8_t      _powerDown[4];
    uint16_t     _values[4];
    uint16_t     _valuesEp[4];
    uint8_t      _intVrefEp[4];
    uint8_t      _gainEp[4];
    uint8_t      _powerDownEp[4];
    static const uint8_t PCA9685_BASE_ADDR                 = 0x40; // 64 - 127 * DeviceID 32-40 overlap with mcp4728
    uint8_t      _pca9685_address;
    uint8_t      _pca9685ID;
    void         pca9685Wake();
    void         pca9685PWMPreScale(uint8_t);
    void         pca9685PWMSingle(uint8_t , uint16_t);
    void         pca9685PWM(uint16_t ,uint16_t ,uint16_t);
    uint16_t     pca9685GetPWM(uint8_t);
    void         HSBtoRGB(uint8_t , uint8_t, uint8_t, uint8_t*, uint8_t*, uint8_t*);
};
#endif

