#include <avr/pgmspace.h>

#include <Wire.h>

byte PROGMEM charArray[36][2] = {
                         {0b1111110, 0b10000100}, // a
                         {0b1111001, 0b11010001}, // b
                         {0b1111, 0b0}, // c
                         {0b111001, 0b11010001}, // d
                         {0b1001111, 0b10000100}, // e
                         {0b1110, 0b10000100}, // f
                         {0b1101111, 0b10000000}, // g
                         {0b1110110, 0b10000100}, // h
                         {0b1001, 0b11010001}, // i
                         {0b110011, 0b0}, // j
                         {0b1100110, 0b10100100}, // k
                         {0b111, 0b0}, // l
                         {0b110110, 0b10101000}, // m
                         {0b10110110, 0b10001000}, // n
                         {0b111111, 0b0}, // o
                         {0b1011110, 0b10000100}, // p
                         {0b10111111, 0b0}, // q
                         {0b11011110, 0b10000100}, // r
                         {0b1101101, 0b10000100}, // s
                         {0b1000, 0b11010001}, // t
                         {0b110111, 0b0}, // u
                         {0b110, 0b10100010}, // v
                         {0b10110110, 0b10000010}, // w
                         {0b10000000, 0b10101010}, // x
                         {0b11010000, 0b10001000}, // y
                         {0b1001, 0b10100010}, // z
                         {0b111111, 0b10100010}, // 0
                         {0b110000, 0b0}, // 1
                         {0b1011011, 0b10000100}, // 2
                         {0b1111001, 0b10000000}, // 3
                         {0b1110100, 0b10000100}, // 4
                         {0b10001101, 0b10000100}, // 5
                         {0b1101111, 0b10000100}, // 6
                         {0b1111000, 0b10000000}, // 7
                         {0b1111111, 0b10000100}, // 8
                         {0b1111101, 0b10000100}, // 9
                         // {0b11111111, 0b11111111}, // ALL ON
                        };

#define TEXTLENGTH 48

signed char scrollRate;
signed char offset;
byte displayLength;
byte displayContents[TEXTLENGTH][2];
byte brightness;

void setup()
{
  byte rVal = 0;

  brightness = 0;
  scrollRate = 1;
  offset = -11;
  displayLength = 0;
  memset(displayContents, 0, sizeof(displayContents));

  Wire.begin(); // Join i2c bus
  Serial.begin(9600);

  Wire.beginTransmission(0b1010000);
  Wire.write(0b10010000); // Display on
  rVal = Wire.endTransmission();

  Serial.print("On: ");
  Serial.println(rVal);

  // Set LED display
  Wire.beginTransmission(0b1010000);
  Wire.write(0b01000001);
  Wire.write(0x07); // LEDs off
  rVal = Wire.endTransmission();

  Serial.print("LED: ");
  Serial.println(rVal);
}

void loop()
{
  static byte iCnt = 0;
  byte rVal;
  byte rSent;
/*
  // Set LED display
  Wire.beginTransmission(0b1010000);
  rSent = Wire.write(0b01000001);
  rSent += Wire.write(iCnt & 0x07);
  rVal = Wire.endTransmission();
*/
  while(Serial.available()) {
    int inChar = Serial.read();

    inChar = tolower(inChar);

    if(inChar == '+') {
      // Set LED display  
      Wire.beginTransmission(0b1010000);
      rSent = Wire.write(0b10010000 | (brightness & 0x0F));
      rVal = Wire.endTransmission();
      if((brightness & 0x0F) != 0x0F) {
        brightness++;
      }
    } else if(inChar == '-') {
      if(brightness != 0) {
        brightness--;
        Wire.beginTransmission(0b1010000);
        rSent = Wire.write(0b10010000 | (brightness & 0x0F));
        rVal = Wire.endTransmission();
      } else {
        Wire.beginTransmission(0b1010000);
        rSent = Wire.write(0b10000000);
        rVal = Wire.endTransmission();
      }
    } else if(inChar == '!') {
      scrollRate = 0;
      offset = 0;
      displayLength = 0;
      memset(displayContents, 0, sizeof(displayContents));
    } else if(inChar == '#') {
      scrollRate = 1;
      offset = -11;
      displayLength = 0;
      memset(displayContents, 0, sizeof(displayContents));
    } else {
      if(displayLength == TEXTLENGTH) {
        break;
      }
  
      if(inChar >= 'a' && inChar <= 'z') {
        memcpy_P(displayContents[displayLength++], ((const byte*)charArray) + (inChar - 'a')*2, sizeof(byte)*2);
      } else if(inChar >= '0' && inChar   <= '9') {
        memcpy_P(displayContents[displayLength++], ((const byte*)charArray) + (inChar - '0' + 26)*2, sizeof(byte)*2);
      } else if(inChar == ' ') {
        memset(displayContents[displayLength++], 0, sizeof(byte)*2);
      }
    }
  }

  // Send address and 12 chars
  Wire.beginTransmission(0b1010000);
    rSent = Wire.write(0b11000000); // Address = 0
    for(signed char iChar = 0; iChar < 12; iChar++) {
      byte char1, char2;
      if((iChar + offset) < 0) {
        char1 = char2 = 0;
      } else if((iChar + offset) < displayLength) {
        char1 = displayContents[iChar + offset][0];
        char2 = displayContents[iChar + offset][1];
      } else {
        char1 = char2 = 0;
      }
      rSent += Wire.write((byte)char1);
      rSent += Wire.write((byte)char2);
      rSent += Wire.write((byte)0);
    }
  rVal = Wire.endTransmission();

  iCnt++;
  offset += scrollRate;

  if(offset >= (displayLength + 6)) {
    offset = -11;
  }

  delay(100);
}
