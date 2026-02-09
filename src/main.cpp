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
// const char *kDisplayText[] = {
//   "Paid was hill sir high. For him precaution any advantages dissimilar comparison few terminated projecting. Prevailed discovery immediate objection of ye at. Repair summer one winter living feebly pretty his. In so sense am known these since. Shortly respect ask cousins brought add tedious nay. Expect relied do we genius is. On as around spirit of hearts genius. Is raptures daughter branched laughter peculiar in settling.",
//   "Abilities forfeited situation extremely my to he resembled. Old had conviction discretion understood put principles you. Match means keeps round one her quick. She forming two comfort invited. Yet she income effect edward. Entire desire way design few. Mrs sentiments led solicitude estimating friendship fat. Meant those event is weeks state it to or. Boy but has folly charm there its. Its fact ten spot drew.",
//   "Piqued favour stairs it enable exeter as seeing. Remainder met improving but engrossed sincerity age. Better but length gay denied abroad are. Attachment astonished to on appearance imprudence so collecting in excellence. Tiled way blind lived whose new. The for fully had she there leave merit enjoy forth.",
//   "His having within saw become ask passed misery giving. Recommend questions get too fulfilled. He fact in we case miss sake. Entrance be throwing he do blessing up. Hearts warmth in genius do garden advice mr it garret. Collected preserved are middleton dependent residence but him how. Handsome weddings yet mrs you has carriage packages. Preferred joy agreement put continual elsewhere delivered now. Mrs exercise felicity had men speaking met. Rich deal mrs part led pure will but.",
//   "Remain lively hardly needed at do by. Two you fat downs fanny three. True mr gone most at. Dare as name just when with it body. Travelling inquietude she increasing off impossible the. Cottage be noisier looking to we promise on. Disposal to kindness appetite diverted learning of on raptures. Betrayed any may returned now dashwood formerly. Balls way delay shy boy man views. No so instrument discretion unsatiable to in.",
//   "Whole every miles as tiled at seven or. Wished he entire esteem mr oh by. Possible bed you pleasure civility boy elegance ham. He prevent request by if in pleased. Picture too and concern has was comfort. Ten difficult resembled eagerness nor. Same park bore on be. Warmth his law design say are person. Pronounce suspected in belonging conveying ye repulsive.",
//   "Style too own civil out along. Perfectly offending attempted add arranging age gentleman concluded. Get who uncommonly our expression ten increasing considered occasional travelling. Ever read tell year give may men call its. Piqued son turned fat income played end wicket. To do noisy downs round an happy books.",
//   "Affronting imprudence do he he everything. Sex lasted dinner wanted indeed wished out law. Far advanced settling say finished raillery. Offered chiefly farther of my no colonel shyness. Such on help ye some door if in. Laughter proposal laughing any son law consider. Needed except up piqued an.",
//   "Savings her pleased are several started females met. Short her not among being any. Thing of judge fruit charm views do. Miles mr an forty along as he. She education get middleton day agreement performed preserved unwilling. Do however as pleased offence outward beloved by present. By outward neither he so covered amiable greater. Juvenile proposal betrayed he an informed weddings followed. Precaution day see imprudence sympathize principles. At full leaf give quit to in they up.",
//   "Promotion an ourselves up otherwise my. High what each snug rich far yet easy. In companions inhabiting mr principles at insensible do. Heard their sex hoped enjoy vexed child for. Prosperous so occasional assistance it discovered especially no. Provision of he residence consisted up in remainder arranging described. Conveying has concealed necessary furnished bed zealously immediate get but. Terminated as middletons or by instrument. Bred do four so your felt with. No shameless principle dependent household do."
// };
// var char* temporaryText = (char*)malloc(256);

U8G2_SH1122_256X64_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/CS_PIN, /* dc=*/DC_PIN, /* reset=*/RST_PIN);
int timer = 0;
const char *kAnimationFrames[] = {"    /|\\    ", "   / | \\   ", "  /  |  \\  ", " /   |   \\ ", "/    |    \\", "\\    |    /", " \\   |   / ", "  \\  |  /  ", "   \\ | /   ", "    \\|/    ", "     X     "};
const int kTotalAnimationFrames = sizeof(kAnimationFrames) / sizeof(kAnimationFrames[0]);
const int kFontHeight = 8 - 2;
const int kFontWidth = 5 - 1;

int speedStage = 3;
const char *kSpeedStageFrames[] = {"[______]", "[#_____]", "[##____]", "[###___]", "[####__]", "[#####_]", "[######]"};

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
  // u8g2.setFont(u8g2_font_5x8_tf); compact 5x8; 7 lines in 64px
  // u8g2.setFont(u8g2_font_courR08_tf); good but bigger 7x11
  // u8g2.setFont(u8g2_font_6x10_tf); 6x10
  // unsigned long timerStart = millis();

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
    for (int i = 0; i < LINES_ON_SCREEN; i++)
    {
      u8g2.drawUTF8(120, (i + 1) * kFontHeight - timer % kFontHeight, kAnimationFrames[(i + timer / kFontHeight) % kTotalAnimationFrames]);
    }
  }

  u8g2.setDrawColor(0);
  u8g2.drawBox(0, 0, 41, 10);
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(0, 8, kSpeedStageFrames[7 - speedStage]);

  timer++;
  if (timer > 1280)
    timer = 0;

  u8g2.sendBuffer();

  if (digitalRead(LEFT_BUTTON) == LOW && !isPressing)
  {
    speedStage = min(7, speedStage + 1);
    Serial.println("speed decreased to: " + String(7 - speedStage));
    isPressing = true;
  }
  else if (digitalRead(RIGHT_BUTTON) == LOW && !isPressing)
  {
    speedStage = max(1, speedStage - 1);
    Serial.println("speed increased to: " + String(7 - speedStage));
    isPressing = true;
  }
  else if (digitalRead(MAIN_BUTTON) == LOW && !isPressing)
  {
    Serial.println("pressing main");
    isRandomPattern = !isRandomPattern;
    u8g2.clearBuffer();
    u8g2.clearDisplay();

    isPressing = true;
  }
  else if (digitalRead(LEFT_BUTTON) == HIGH && digitalRead(RIGHT_BUTTON) == HIGH)
  {
    isPressing = false;
  }

  // Serial.println("Loop duration: " + String(millis() - timerStart) + " ms");

  if (speedStage != 7)
  {
    delay(speedStage * speedStage * 5);
  }
}