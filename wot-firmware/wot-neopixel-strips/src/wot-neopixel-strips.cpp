#include "application.h"
#include "soundtocolor.h"
#include "neopixel.h"

#include "JsonParserGeneratorRK.h"
JsonParserStatic<2048, 100> jsonParser;

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_PIN D2

// wot-strip-1 and 3 have 60 lights, 2 and 4 have 30
#define PIXEL_COUNT 30 // NUM of lights

#define PIXEL_TYPE WS2812B
#define BRIGHTNESS 150 // 0 - 255

#define CHASE_DELAY 30

// Pub/Sub strings for each device

// For wot-strip-1
// #define SUB_STRING "startOne"
// #define PUB_STRING "startTwo"

// For wot-strip-2
// #define SUB_STRING "startTwo"
// #define PUB_STRING "startThree"

// For wot-strip-3
// #define SUB_STRING "startThree"
// #define PUB_STRING "startFour"

// For wot-strip-4
#define SUB_STRING "startFour"
#define PUB_STRING "startOne"

// Strip One is also mounted backwards, so it needs to count up, not down
// #define ITERATE_REVERSE true
// Set for strips 2-4
#define ITERATE_REVERSE false

/* OPTIONS 
* 0 = RANDOM
* 1 = COLOR_OF_SOUND
* 2 = TRELLIS
* 3 = TRELLIS_PIXEL
* 4 = RAINBOW
* 5 = CHASE
*/
int animationMode = 0;
bool useWheel = false;
int wheelPos;

Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

// Cyan by default
int redValue = 0;
int greenValue = 255;
int blueValue = 255;

bool lightUp = false;

void rainbow(uint8_t wait);
uint16_t Wheel(byte WheelPos);

void playColorOfSound(const char *data)
{
  char *ptr;
  double freq = strtod(data, &ptr);

  useWheel = false;

  if (data && freq != 0)
  {
    struct color noteColors;

    noteColors = mapNoteToColor(freq);

    redValue = noteColors.Red;
    greenValue = noteColors.Green;
    blueValue = noteColors.Blue;
  }
  else
  {
    greenValue = random(1, 256);
    redValue = random(1, 256);
    blueValue = random(1, 256);
  }
}

void setColor(const char *event, const char *data)
{
  // Split the comma-delimited list into RGB values
  jsonParser.clear();
  jsonParser.addString(data);

  if (jsonParser.parse())
  {
    // set to R, G and B
    redValue = jsonParser.getReference().key("red").valueInt();
    greenValue = jsonParser.getReference().key("green").valueInt();
    blueValue = jsonParser.getReference().key("blue").valueInt();
  }
  else
  {
    Particle.publish("parse-fail", String(data));
  }
}

void playTrellis(const char *data)
{
  wheelPos = atoi(data);

  useWheel = true;
}

void playRandom()
{
  useWheel = false;

  greenValue = random(1, 256);
  redValue = random(1, 256);
  blueValue = random(1, 256);
}

void playTone(const char *event, const char *data)
{
  lightUp = true;

  switch (animationMode)
  {
  case 0:
    playRandom();
    break;
  case 1:
    playColorOfSound(data);
    break;
  case 2:
    playTrellis(data);
    break;
  default:
    playRandom();
    break;
  }
}

void stopTone(const char *event, const char *data)
{
  lightUp = false;
}

void setAllPixels(u_int16_t red, u_int16_t green, u_int16_t blue)
{
  for (int i = 0; i < strip.numPixels(); i++)
  {
    if (useWheel && lightUp)
    {
      strip.setPixelColor(i, Wheel(wheelPos));
    }
    else
    {
      strip.setPixelColor(i, red, green, blue);
    }
  }
  strip.show();
}

int setModeCloud(String args)
{
  int mode = args.toInt();

  animationMode = mode;

  return 1;
}

void setModeMesh(const char *event, const char *data)
{
  int mode = String(data).toInt();

  animationMode = mode;
}

void chase(uint8_t wait)
{
  if (ITERATE_REVERSE)
  {
    for (uint16_t i = strip.numPixels(); i > 0; i--)
    {
      strip.setPixelColor(i, strip.Color(redValue, greenValue, blueValue));

      strip.show();
      delay(wait);
    }
  }
  else
  {
    for (uint16_t i = 0; i < strip.numPixels(); i++)
    {
      strip.setPixelColor(i, strip.Color(redValue, greenValue, blueValue));

      strip.show();
      delay(wait);
    }
  }
}

void chaseOff(uint8_t wait)
{
  if (ITERATE_REVERSE)
  {
    for (uint16_t i = strip.numPixels(); i > 0; i--)
    {
      strip.setPixelColor(i, 0, 0, 0); // Off

      strip.show();
      delay(wait);
    }
  }
  else
  {
    for (uint16_t i = 0; i < strip.numPixels(); i++)
    {
      strip.setPixelColor(i, 0, 0, 0); // Off

      strip.show();
      delay(wait);
    }
  }
}

void startStrip(const char *event, const char *data)
{
  if (animationMode == 5)
  {
    chase(CHASE_DELAY);

    Mesh.publish(PUB_STRING, NULL);

    delay(50);

    chaseOff(CHASE_DELAY);
  }
}

void setup()
{
  Serial.begin(9600);

  strip.setBrightness(BRIGHTNESS);
  strip.begin();
  strip.show();

  setAllPixels(0, 0, 255);
  delay(1000);

  Mesh.subscribe("tone", playTone);
  Mesh.subscribe("no-tone", stopTone);

  Particle.function("setMode", setModeCloud);
  Particle.variable("mode", animationMode);

  Mesh.subscribe(SUB_STRING, startStrip);
  Mesh.subscribe("setMode", setModeMesh);
  Mesh.subscribe("setColor", setColor);
}

void loop()
{
  if (animationMode != 4 && animationMode != 5)
  {
    if (!lightUp)
    {
      setAllPixels(0, 0, 0);
    }
  }
  else if (animationMode == 4)
  {
    rainbow(20);
  }
}

void rainbow(uint8_t wait)
{
  uint16_t i, j;

  for (j = 0; j < 256; j++)
  {
    for (i = 0; i < strip.numPixels(); i++)
    {
      strip.setPixelColor(i, Wheel((i + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Set all pixels in the strip to a solid color, then wait (ms)
void colorAll(uint32_t c, uint8_t wait)
{
  uint16_t i;

  for (i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, c);
  }
  strip.show();
  delay(wait);
}

uint16_t Wheel(byte WheelPos)
{
  WheelPos = 255 - WheelPos;

  if (WheelPos < 85)
  {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }

  if (WheelPos < 170)
  {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }

  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}