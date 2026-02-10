#include <string>
#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>

// screen drawing take around 216ms at 8MHz, 103ms at 16MHz, 50ms at 32MHz, 22ms at 92MHz
#define CPU_MHZ 96 // min 32, default 96
#define DISPLAY_WIDTH 255
#define DISPLAY_HEIGHT 64
#define LINES_ON_SCREEN 12
#define NUMBER_OF_STARS 8

#define SCL_PIN 4  // clock pin
#define SDA_PIN 5  // data pin, MOSI pin
#define RST_PIN 10 // reset pin; free GPIO
#define DC_PIN 11  // data/command pin; free GPIO
#define CS_PIN 1   // chip select pin

#define LEFT_BUTTON 12
#define RIGHT_BUTTON 14
#define MAIN_BUTTON 9

enum Direction
{
  LEFT = '/',
  DOWN = '|',
  RIGHT = '\\',
};

struct FallingStar
{
  Direction direction = DOWN;
  int position = 0;
  char symbol = '|';
};

U8G2_SH1122_256X64_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/CS_PIN, /* dc=*/DC_PIN, /* reset=*/RST_PIN);
int timer = 0;
const char *kAnimationFrames[] = {"    /|\\    ",
                                  "   / | \\   ",
                                  "  /  |  \\  ",
                                  " /   |   \\ ",
                                  "/    |    \\",
                                  "\\    |    /",
                                  " \\   |   / ",
                                  "  \\  |  /  ",
                                  "   \\ | /   ",
                                  "    \\|/    ",
                                  "     X     "};
const int kTotalAnimationFrames = sizeof(kAnimationFrames) / sizeof(kAnimationFrames[0]);
const int kFontHeight = 8 - 2;
const int kFontWidth = 5 - 1;

int speedStage = 3;
const char *kSpeedStageFrames[] = {"[######]", "[_#####]", "[__####]",
                                   "[___###]", "[____##]", "[_____#]",
                                   "[______]"};

FallingStar stars[NUMBER_OF_STARS][LINES_ON_SCREEN] = {};
bool isPressing = false;
bool isRandomPattern = false;
int mutationChance = 5;

void setup()
{
  Serial.begin(115200);

  delay(1000);

  setCpuFrequencyMhz(CPU_MHZ);

  SPI.begin(SCL_PIN, -1, SDA_PIN, -1);

  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2.setFont(u8g2_font_5x8_tf);
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  pinMode(MAIN_BUTTON, INPUT_PULLUP);
  int starsSpacing = DISPLAY_WIDTH / NUMBER_OF_STARS;
  for (int i = 0; i < NUMBER_OF_STARS; i++)
  {
    for (int j = 0; j < LINES_ON_SCREEN; j++)
    {
      stars[i][j].position = i * starsSpacing;
    }
  }
}

void loop()
{
  // -- good small fonts --
  // u8g2.setFont(u8g2_font_5x8_tf); //compact; 5x8
  // u8g2.setFont(u8g2_font_6x10_tf); //more clear; 6x10
  // u8g2.setFont(u8g2_font_courR08_tf); //courier new; 7x11
  // u8g2.setFont(u8g2_font_timR08_tf); //times new roman; 7x11

  u8g2.clearBuffer();

  if (isRandomPattern)
  {
    if (timer % kFontHeight == 0)
    {
      // push the stars down
      for (int i = 0; i < NUMBER_OF_STARS; i++)
      {
        for (int j = 0; j < LINES_ON_SCREEN - 1; j++)
        {
          stars[i][j].direction = stars[i][j + 1].direction;
          stars[i][j].position = stars[i][j + 1].position;
        }
      };

      // make new stars below
      for (int i = 0; i < NUMBER_OF_STARS; i++)
      {
        FallingStar &previousStar = stars[i][LINES_ON_SCREEN - 1];

        if (random() % 101 < mutationChance)
        {
          switch (random() % 3)
          {
          case 0:
            stars[i][LINES_ON_SCREEN - 1].direction = LEFT;
            break;
          case 1:
            stars[i][LINES_ON_SCREEN - 1].direction = DOWN;
            break;
          default:
            stars[i][LINES_ON_SCREEN - 1].direction = RIGHT;
          }
        }

        switch (previousStar.direction)
        {
        case LEFT:
          stars[i][LINES_ON_SCREEN - 1].position -= kFontWidth;
          if (stars[i][LINES_ON_SCREEN - 1].position < 0)
          {
            stars[i][LINES_ON_SCREEN - 1].position = 0;
            stars[i][LINES_ON_SCREEN - 1].direction = RIGHT;
            break;
          }
          stars[i][LINES_ON_SCREEN - 1].direction = previousStar.direction;
          break;
        case RIGHT:
          stars[i][LINES_ON_SCREEN - 1].position += kFontWidth;
          if (stars[i][LINES_ON_SCREEN - 1].position > DISPLAY_WIDTH - kFontWidth)
          {
            stars[i][LINES_ON_SCREEN - 1].position -= kFontWidth;
            stars[i][LINES_ON_SCREEN - 1].direction = LEFT;
            break;
          }
          stars[i][LINES_ON_SCREEN - 1].direction = previousStar.direction;
          break;
        default:
          stars[i][LINES_ON_SCREEN - 1].direction = previousStar.direction;
        }
      };
    }

    // draw stars
    for (int i = 0; i < NUMBER_OF_STARS; i++)
    {
      for (int j = 0; j < LINES_ON_SCREEN; j++)
      {
        u8g2.drawUTF8(stars[i][j].position, (j + 1) * kFontHeight - timer % kFontHeight, std::string(1, stars[i][j].direction).c_str());
      }
    }
  }
  else
  {
    // draw pattern
    for (int i = 0; i < LINES_ON_SCREEN; i++)
    {
      u8g2.drawUTF8(120, (i + 1) * kFontHeight - timer % kFontHeight, kAnimationFrames[(i + timer / kFontHeight) % kTotalAnimationFrames]);
    }
  }
  timer++;
  if (timer > 1280)
    timer = 0;

  // draw speed stage
  u8g2.setDrawColor(0);
  u8g2.drawBox(0, 0, 41, 10);
  u8g2.setDrawColor(1);
  u8g2.drawGlyph(0, 8, '[');
  u8g2.drawGlyph(27, 8, ']');
  u8g2.drawBox(1, 3, 5 * (6 - speedStage), 4);

  u8g2.sendBuffer();

  // buttons
  if (digitalRead(LEFT_BUTTON) == LOW && !isPressing)
  {
    speedStage = min(6, speedStage + 1);
    Serial.println("delay factor increased to: " + String(speedStage));
    isPressing = true;
  }
  else if (digitalRead(RIGHT_BUTTON) == LOW && !isPressing)
  {
    speedStage = max(0, speedStage - 1);
    Serial.println("delay factor decreased to: " + String(speedStage));
    isPressing = true;
  }
  else if (digitalRead(MAIN_BUTTON) == LOW && !isPressing)
  {
    Serial.println("pressing main");
    isRandomPattern = !isRandomPattern;
    u8g2.clearBuffer();
    u8g2.clearDisplay();
    delay(500);

    isPressing = true;
  }
  else if (digitalRead(LEFT_BUTTON) == HIGH && digitalRead(RIGHT_BUTTON) == HIGH)
  {
    isPressing = false;
  }

  if (speedStage != 0)
  {
    delay(speedStage * speedStage * 5);
  }
}