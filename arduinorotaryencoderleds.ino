#include <FastLED.h>
#define NUM_LEDS 15
#define DATA_PIN 6
#define ROTARY_ENCODER_DT_PIN 2
#define ROTARY_ENCODER_CLT_PIN 3
#define ROTARY_ENCODER_BUTTON_PIN 7
CRGB leds[NUM_LEDS];

bool savedRotaryEncoderDtValue;
long unsigned rotaryEncoderSecureTimer;

bool isButtonPressed = false;

int positionIndex = 0;
int range = 1;
bool isRangeMode = false;

bool firstLoop = true;

enum positionStatus
{
  UNCHANGED,
  CHANGED,
  ERROR
};
long unsigned showErrorTimer = 0;
bool errorStatus = false;

void setup()
{
  pinMode(ROTARY_ENCODER_BUTTON_PIN, INPUT);

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(9600);

  savedRotaryEncoderDtValue = digitalRead(ROTARY_ENCODER_DT_PIN);

  rotaryEncoderSecureTimer = millis();

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
}

void loop()
{
  bool modeChanged = manageSwitchMode();

  enum positionStatus status = managePositionChange();

  bool showError = manageErrorVisibility(status);

  if (modeChanged || status != UNCHANGED || firstLoop || errorStatus != showError)
  {
    serialPrintInfo(positionIndex, range);

    renderLeds(positionIndex, range, isRangeMode, showError);

    firstLoop = false;
    errorStatus = showError;
  }
}

bool manageErrorVisibility(enum positionStatus status)
{
  if (status == ERROR)
  {
    showErrorTimer = millis();
  }

  if (showErrorTimer != 0 && abs(millis() - showErrorTimer) < 100)
  {
    return true;
  }
  else if (showErrorTimer != 0)
  {
    showErrorTimer = 0;
  }

  return false;
}

bool manageSwitchMode()
{
  bool buttonPressed = getButtonPressed();

  if (buttonPressed)
  {
    isRangeMode = !isRangeMode;

    return true;
  }

  return false;
}

bool getButtonPressed()
{
  int buttonState = digitalRead(ROTARY_ENCODER_BUTTON_PIN);

  if (buttonState == HIGH)
  {
    isButtonPressed = true;
  }
  else if (isButtonPressed)
  {
    isButtonPressed = false;

    return true;
  }

  return false;
}

enum positionStatus managePositionChange()
{
  bool success = false;
  int movement = getRotaryEncoderPosition();

  if (movement == 0)
  {
    return UNCHANGED;
  }

  if (isRangeMode)
  {
    success = setGlobalRange(movement, positionIndex, NUM_LEDS);
  }
  else
  {
    success = setGlobalPositionIndex(movement, range, NUM_LEDS);
  }

  return success ? CHANGED : ERROR;
}

bool setGlobalPositionIndex(int movement, int currentRange, int numLed)
{
  int newValue = positionIndex + movement;

  if (newValue < 0 || newValue + currentRange > numLed)
  {
    Serial.println("Error bad position");

    return false;
  }

  positionIndex = newValue;

  return true;
}

bool setGlobalRange(int movement, int currentPositionIndex, int numLed)
{
  int newValue = range + movement;

  if (newValue < 1 || newValue + currentPositionIndex > numLed)
  {
    Serial.println("Error bad range");

    return false;
  }

  range = newValue;

  return true;
}

int getRotaryEncoderPosition()
{
  bool dtStatus = digitalRead(ROTARY_ENCODER_DT_PIN);
  int movement = 0;

  if (dtStatus != savedRotaryEncoderDtValue)
  {
    if (abs(millis() - rotaryEncoderSecureTimer) > 50)
    {
      if (digitalRead(ROTARY_ENCODER_CLT_PIN) != savedRotaryEncoderDtValue)
      {
        movement = -1;
      }
      else
      {
        movement = 1;
      }
      rotaryEncoderSecureTimer = millis();
    }
    savedRotaryEncoderDtValue = dtStatus;
  }
  return movement;
}

void serialPrintInfo(int currentPositionIndex, int currentRange)
{
  Serial.print("Position :");
  Serial.print(currentPositionIndex);
  Serial.print("  Range :");
  Serial.println(currentRange);
}

void renderLeds(int startIndex, int range, bool isRangeMode, bool error)
{
  for (int j = startIndex; j < startIndex + range; j++)
  {

    leds[j] = getLedColor(isRangeMode, error);
  }
  FastLED.show();
  FastLED.clear();
}

CRGB getLedColor(bool isRangeMode, bool error)
{
  CRGB color = CRGB::Green;

  if (error)
  {
    color = CRGB::Red;
  }
  else if (isRangeMode)
  {
    color = CRGB::White;
  }

  return color;
}
