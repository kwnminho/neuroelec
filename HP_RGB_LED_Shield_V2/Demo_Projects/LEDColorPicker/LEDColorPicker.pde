#include <Wire.h>
#include <HPRGB2.h>

HPRGB ledShield; // default mcp4728 id(0) and default PCA9685 id(0)

// Indicates what color we are reading next; 0 = red, 1 = green, 2 = blue
int pos = 0;

// red PWM value
int red = 0;
// green PWM value
int green = 0;
// blue PWM value
int blue = 0;

// indicates if next byte should be unescaped
boolean escape = false;

void setup() {
  Serial.begin(9600);
  ledShield.begin();
}

void loop () {
    
    while (Serial.available()) {
      int rgb = Serial.read();
      
      if (rgb == 1) {
        // end of RGB sequence byte
        // reset pos
        pos = 0;
    
        // process this color
        ledShield.goToRGB(red,green,blue);
    
        // Send ACK byte so Java app can send the next color
        Serial.print("k");
        Serial.flush();
        
        // get next byte
        continue;
      } else if (rgb == 2) {
        // escape byte
        escape = true;
        // discard byte and read next byte
        continue;
      }
      
      if (escape) {
        // unescape byte
        rgb = 0x20 ^ rgb;
        // reset escape
        escape = false;  
      }
      
      switch (pos++) {
        case 0:
          red = rgb;
          break;
        case 1:
          green = rgb;
          break;
        case 2:
          blue = rgb;
          break;
      }
    }
}

