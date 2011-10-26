#include <Wire.h>

void setup()

{
  Wire.begin(); // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
}


int readAllRDAC()
{
  Wire.beginTransmission(0x2c); // transmit to device U1 (0x2c or hex for 0101100)  
  Wire.write((byte)0x00);
  Wire.endTransmission();
  Wire.requestFrom(0x2c, 4);
  int rdacnum = 0;
  while(Wire.available())    // slave may send less than requested
  { 
    byte rdac = Wire.read(); // receive a byte as character
    Serial.print("RDAC");
    Serial.print(rdacnum,DEC);  
    Serial.print(" = "); 
    Serial.print(rdac,DEC);         // print the character
    Serial.print(" ");  
    rdacnum++; 
  }
  Serial.println(" ");
}
byte readRDAC(int address)
{
  Wire.beginTransmission(0x2c); // transmit to device U1 (0x2c or hex for 0101100)  
  Wire.write(address);
  Wire.endTransmission();
  Wire.requestFrom(0x2c, 1);
  byte rdac;
  if(Wire.available()) {   // slave may send less than requested
    rdac = Wire.read(); // receive a byte as character
  }
  return (rdac);
}
void readAllEEMEM()
{
  Wire.beginTransmission(0x2c); // transmit to device U1 (0x2c or hex for 0101100)  
  Wire.write((byte)0x20);
  Wire.endTransmission();
  Wire.requestFrom(0x2c, 4);
  int eemnum = 0;
  while(Wire.available())    // slave may send less than requested
  { 
    byte eem = Wire.read(); // receive a byte as character
    Serial.print("EEMEM");
    Serial.print(eemnum,DEC);  
    Serial.print(" = "); 
    Serial.print(eem,DEC);         // print the character
    Serial.print(" ");  
    eemnum++;
  }
  Serial.println(" ");  
}
byte readEEMEM(int address)
{
  address = address + 32;
  Wire.beginTransmission(0x2c); // transmit to device U1 (0x2c or hex for 0101100)  
  Wire.write(address);
  Wire.endTransmission();
  Wire.requestFrom(0x2c, 1);
  byte eem;
  while(Wire.available())    // slave may send less than requested
  { 
    eem = Wire.read(); // receive a byte as character
  }
  return (eem);
}
void RDACWrite(int address, int value) {
  Wire.beginTransmission(0x2c);
  Wire.write(address);            // sends instruction byte to RDAC1 (0x01 or hex for 0000001)
  Wire.write(value);
  Wire.endTransmission();
}
void loop()
{

readAllRDAC();
byte rdac = readRDAC(1);
Serial.println(rdac,DEC);

readAllEEMEM();
byte eem = readEEMEM(1);
Serial.println(eem,DEC);

delay(4000);  // time delay to see resistance values change via multimete
}
