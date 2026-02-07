#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>

#define SCL_PIN 4  // clock pin
#define SDA_PIN 5  // data pin, MOSI pin
#define RST_PIN 10 // reset pin; free GPIO
#define DC_PIN 11  // data/command pin; free GPIO
#define CS_PIN 1   // chip select pin

U8G2_SH1122_256X64_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/SCL_PIN, /* data=*/SDA_PIN, /* cs=*/CS_PIN, /* dc=*/DC_PIN, /* reset=*/RST_PIN);

void setup()
{
  Serial.begin(115200);

  delay(1000);

  u8g2.begin();
  pinMode(13, OUTPUT);
}

void loop()
{
  u8g2.clearBuffer();   // clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr);  // choose a suitable font
  u8g2.drawStr(0, 0, "Lorem ipsum dolor sit amet, consectetur adipiscing"); 
  u8g2.drawStr(0, 20, "scing elit, sed do eiusmod tempor incididunt ut labor");
  u8g2.drawStr(0, 30, "dolore magna aliqua. Ut enim ad minim veniam, quis nos");
  u8g2.drawStr(0, 35, "trud exercitation ullamco laboris nisi ut aliquip ex");
  u8g2.sendBuffer();  // transfer internal memory to the display
  // delay(10000);
}
