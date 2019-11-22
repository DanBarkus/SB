#include <Wire.h>
#include <SPI.h>
// #include <EEPROM.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPRLS.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_NeoPixel.h>

// #include <animation.hpp>

#define LIS3DH_CS 5
#define PIN 12
#define VALVE 10
#define SLEEPPIN 6
#define VBATPIN A7

// Button Definitions
#define UP_BUTTON A2
#define DOWN_BUTTON A3
#define CENTER_BUTTON A1

#define INTERRUPTPIN 0

#define NUM_LEDS 8
#define BRIGHTNESS 18
#define NUMBRIGHTNESS 1

#define CLICKTHRESHHOLD 21
#define TIMELATENCY 2
#define TIMELIMIT 15
#define TIMEWINDOW 20

#define RESET_PIN -1 // set to any GPIO pin # to hard-reset on begin()
#define EOC_PIN -1   // set to any GPIO pin to read end-of-conversion by pin
Adafruit_MPRLS mpr = Adafruit_MPRLS(RESET_PIN, EOC_PIN);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRBW + NEO_KHZ800);

Adafruit_LIS3DH lis = Adafruit_LIS3DH(LIS3DH_CS);

byte colorCycle[7][4] = {
    {0, 40, 0, 0}, {0, 0, 40, 0}, {40, 0, 0, 0}, {40, 0, 40, 0}, {0, 40, 40, 0}, {60, 20, 0, 0}, {0, 0, 0, 40}};

float initReading = 0;
float targetReading = 0;

const int offset = 300;
const int pullThresh = 12;

const int numReadings = 10;
float readings[numReadings]; // the readings from the analog input
int readIndex = 0;           // the index of the current reading
float total = 0;             // the running total
float average = 0;           // the average

float score = 0;

float timeRatio = 0.0035;
const float scoreInc = 0.003;
//score += (rate * scoreInc) + deltaTime * timeRatio;

unsigned long lastActiveTime = 0;
unsigned long timeout = 4 * 1000;

unsigned long deltaTime = 0;

unsigned long lastSleepTime = 0;
unsigned long powerdown = 120 * 1000;

int scoreThreshold = 12;
float scorePreOpen = 0.5;

unsigned long lastOpenTime = 0;
unsigned long open = 1.2 * 1000;
unsigned long hold = 4 * 1000;
unsigned long close = 6.5 * 1000;
unsigned int openPower = 200;
unsigned int holdPower = 50;

bool sleeping = false;
bool recording = false;
volatile bool interrupted = false;
unsigned int opValve = 0;

int saveInterval = 1 * 1000; // One second
int lastSave = 0;

int filenum = 1;

// animation params
float animTimeout[NUM_LEDS];
bool animComplete[NUM_LEDS];

// menu params
int lastUp = 1;
int lastDown = 1;
int lastCenter = 1;
int currButton = 0;
int menuTime = 1000;
unsigned long lastButtonTime = 0;
bool menuOpen = false;

void setup()
{
  Serial.begin(9600);

  pinMode(SLEEPPIN, OUTPUT);
  pinMode(VALVE, OUTPUT);
  pinMode(UP_BUTTON, INPUT_PULLUP);
  pinMode(DOWN_BUTTON, INPUT_PULLUP);
  pinMode(CENTER_BUTTON, INPUT_PULLUP);
  digitalWrite(SLEEPPIN, HIGH);
  digitalWrite(VALVE, LOW);
  delay(20);
  strip.setBrightness(BRIGHTNESS);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  Serial.println(F("BMP280 test"));
  if (!mpr.begin())
  {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    for (int i = 0; i < 9; i++)
    {
      strip.setPixelColor(i, strip.Color(20, 0, 0, 0));
      strip.show();
    }
  }

  if (!lis.begin(0x18))
  { // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    strip.setPixelColor(10, strip.Color(20, 0, 0, 0));
    strip.show();
    delay(1000);
    strip.clear();
    strip.show();
  }
  else
  {
    strip.setPixelColor(11, strip.Color(0, 40, 0, 0));
    strip.show();
    lis.setRange(LIS3DH_RANGE_2_G); // 2, 4, 8 or 16 G!
    lis.setDataRate(LIS3DH_DATARATE_50_HZ);
    lis.setClick(1, CLICKTHRESHHOLD, TIMELIMIT, TIMELATENCY, TIMEWINDOW);
    Serial.println("LIS3DH found!");
  }

  // initialize all the readings to 0:
  // for (int thisReading = 0; thisReading < numReadings; thisReading++)
  // {
  //   readings[thisReading] = 0;
  // }

  strip.clear();
  strip.show();

  // if (EEPROM.length() == 0)
  // {
  //   EEPROM.write(0, 1);
  // }
  // filenum = EEPROM.read(0);

  for (int i = 0; i < 20; i++)
  {
    getInit();
  }
  checkBattery();
  lastSleepTime = millis();
  lastActiveTime = millis();
}

// ---------------------------------------------------------------------------------

void loop()
{
  // Get current time
  unsigned long hTime = millis();
  if (sleeping)
  {
    // Serial.println("Sleeping");
    // if the wake interrupt is triggered
    if (interrupted)
    {
      // turn everything back on
      detachInterrupt(INTERRUPTPIN);
      digitalWrite(SLEEPPIN, HIGH);
      delay(20);
      interrupted = false;
      sleeping = false;
      Serial.println("Starting Bar Display");
      strip.setBrightness(BRIGHTNESS);
      strip.begin();
      // resetGame();

      Serial.println("Starting Pressure Sensor");
      if (!mpr.begin())
      {
        Serial.println("Could not find a valid BMP280 sensor, check wiring!");
        for (int i = 0; i < 9; i++)
        {
          strip.setPixelColor(i, strip.Color(20, 0, 0, 0));
          strip.show();
        }
        // while (1);
      }

      for (int i = 0; i < 10; i++)
      {
        getInit();
      }
      checkBattery();
      hTime = millis();
      lastSleepTime = millis();
      lastActiveTime = millis();
    }
  }
  else
  {
    int currButton = checkButtons();
    int reading = getNextReading();
    bool hitting = checkHitting(reading);
    // updatePressureBar(reading);
    if (opValve == 0)
    {
      updateProgressBar(score);
    }
    Serial.println(initReading);
    Serial.println(targetReading);
    Serial.println(reading);
    strip.show();
    float rate = getRate(reading);
    if (hitting && opValve == 0)
    {
      // Serial.println(rate);
      deltaTime = hTime - lastActiveTime;
      if (deltaTime > 200)
      {
        deltaTime = 200;
      }
      score = updateScore(rate);
      opValve = checkValveScore(score, opValve);
      lastActiveTime = hTime;
      lastSleepTime = hTime;
      recording = true;
      writeReading(hTime, reading, rate, score, hitting);
    }
    else if (opValve == 0)
    {
      if (checkScoreTimeout(hTime))
      {
        if (recording)
        {
          filenum++;
          // EEPROM.write(0, filenum);
          recording = false;
        }
        resetGame();
      }
      else if (checkSleepTimeout(hTime))
      {
        strip.clear();
        strip.show();
        digitalWrite(SLEEPPIN, LOW);
        sleeping = true;
        total = 0;
        attachInterrupt(digitalPinToInterrupt(INTERRUPTPIN), wake, RISING);
        interrupted = false;
      }
      else
      {
        writeReading(hTime, reading, rate, score, hitting);
      }
    }
    opValve = openValve(opValve, hTime, reading);
    updateTgtScore(currButton, hitting);
  }
}

// ---------------------------------------------------------------------------------

int getNextReading()
{
  total = total - readings[readIndex];
  readings[readIndex] = mpr.readPressure() * 30.0;
  // Serial.println(mpr.readPressure());
  total = total + readings[readIndex];
  readIndex = readIndex + 1;

  if (readIndex >= numReadings)
  {
    readIndex = 0;
  }

  average = total / numReadings;
  // delay(10); // delay in between reads
  return average;
}

void updatePressureBar(int reading)
{
  for (int i = 9; i < 18; i++)
  {
    strip.setPixelColor(i, strip.Color(0, 0, 0, 0));
  }
  int off = abs(reading - targetReading);
  // Serial.println(off);
  int pos = int(off / ((offset - pullThresh) / 5));
  if (reading > targetReading)
  {
    if (pos >= 4)
    {
      strip.setPixelColor(17, strip.Color(0, 0, 0, 40));
    }
    else
    {
      strip.setPixelColor(13 + pos, strip.Color(200 - pos * 50, 200 - pos * 50, 0, 40));
    }
  }
  else if (reading < targetReading)
  {
    if (pos >= 4)
    {
      strip.setPixelColor(9, strip.Color(200 + (pos - 4) * 10, 0, 0, 40 - (10 * (pos - 4))));
    }
    else
    {
      strip.setPixelColor(13 - pos, strip.Color(pos * 50, 0, 0, 40));
    }
  }
  if (pos == 0)
  {
    strip.setPixelColor(13, strip.Color(0, 150, 0, 40));
  }
}

float getRate(int reading)
{
  float rate = abs(reading - initReading + 20.0);
  //  Serial.println(rate);
  rate = rate / 35;
  if (rate > 20)
  {
    rate = 20;
  }
  return rate;
}

bool checkHitting(int reading)
{
  if (reading - initReading > -pullThresh)
  {
    return false;
  }
  else
  {
    return true;
  }
}

float updateScore(float rate)
{
  // return score += (rate * scoreInc) * timeRatio + scoreInc;
  return score += (rate * scoreInc) + deltaTime * timeRatio;
}

void updateProgressBar(float score)
{
  int barTop = scaleDisplay(score, scoreThreshold);
  strip.setPixelColor(0, strip.Color(0, 40, 0, 0));
  for (int i = 0; i < barTop; i++)
  {
    strip.setPixelColor(i, strip.Color(0, 40, 0, 0));
  }
}

void resetGame()
{
  if (score > 0)
  {
    score = 0;
    strip.clear();
  }
  getInit();
}

// 0 - ready to be opened
// 1 - opening
// 4 - holding open
// 2 - cooling down
// 3 - triggered to open
int openValve(unsigned int opening, unsigned long milli, int reading)
{
  Serial.println(opening);
  if (opening == 2)
  {
    if (milli - lastOpenTime < close)
    {
      analogWrite(VALVE, 0);
      int numLights = scaleDisplay(milli - lastOpenTime, close);
      for (int i = 0; i < numLights; i++)
      {
        strip.setPixelColor(i, strip.Color(0, 0, 0, 0));
      }
      for (int i = NUM_LEDS; i > NUM_LEDS - abs(numLights - NUM_LEDS); i--)
      {
        strip.setPixelColor(i, strip.Color(40, 0, 0, 0));
      }
      return 2;
    }
    else
    {
      strip.clear();
      strip.show();
      return 0;
    }
  }
  else if (opening == 3)
  {
    lastOpenTime = milli;
    return 1;
  }
  else if (opening == 1)
  {
    if (milli - lastOpenTime < open)
    {
      Serial.println("Opening");
      for (int i = 0; i < 8; i++)
      {
        strip.setPixelColor(i, strip.Color(30, 15, 3, 0));
      }
      analogWrite(VALVE, openPower);
      return 1;
    }
    else
    {
      lastOpenTime = milli;
      return 4;
    }
  }
  else if (opening == 4)
  {
    if (reading == initReading + 1)
    {
      lastOpenTime = milli;
      return 2;
    }
    else if (milli - lastOpenTime < hold)
    {
      Serial.println("Holding");
      analogWrite(VALVE, holdPower);
      return 4;
    }
    else
    {
      lastOpenTime = milli;
      return 2;
    }
  }
  else
  {
    return 0;
  }
  strip.show();
}

int checkValveScore(float score, int opValve)
{
  if (score > scoreThreshold - scorePreOpen && opValve == 0)
  {
    return 3;
  }
  else
  {
    return opValve;
  }
}

bool checkScoreTimeout(unsigned long milli)
{
  if (milli - lastActiveTime > timeout)
  {
    lastActiveTime = milli;
    return true;
  }
  return false;
}

bool checkSleepTimeout(unsigned long milli)
{
  if (milli - lastSleepTime > powerdown)
  {
    return true;
  }
  return false;
}

int checkBattery()
{
  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
  float battBars = measuredvbat - 3.3;
  battBars *= 9;
  Serial.print("VBat: ");
  Serial.println(measuredvbat);
  int red = (3 - battBars);
  if (red < 0)
  {
    red = 0;
  }
  else
  {
    red = 30;
  }
  int green = (battBars - 4);
  if (green < 0)
  {
    green = 0;
  }
  else
  {
    green = 30;
  }

  uint32_t batteryColor = strip.Color(red, green, 0, 0);

  for (int i = 0; i < NUM_LEDS; i++)
  {
    strip.setPixelColor(i, strip.Color(0, 0, 0, 13));
  }
  for (int i = 0; i < battBars; i++)
  {
    strip.setPixelColor(i, batteryColor);
  }
  strip.show();
  delay(500);
  strip.clear();
}

int scaleDisplay(float increment, int total)
{
  int output = round(increment * NUM_LEDS / total);
  return output;
}

// 0 - none
// 1 - up
// 2 - down
// 3 - center
// 4 - up + down
int checkButtons()
{
  int up = digitalRead(UP_BUTTON);
  // Serial.println(up);
  int down = digitalRead(DOWN_BUTTON);
  int center = digitalRead(CENTER_BUTTON);
  int currButton = 0;
  if ((up == 0 && down == 0) && (lastUp != 0 && lastDown != 0))
  {
    currButton = 4;
  }
  else if (up == 0 && lastUp != 0)
  {
    currButton = 1;
  }
  else if (down == 0 && lastDown != 0)
  {
    currButton = 2;
  }
  else if (center == 0 && lastCenter != 0 && lastUp != 0 && lastDown != 0)
  {
    currButton = 3;
  }
  else
  {
    currButton = 0;
  }
  lastUp = up;
  lastDown = down;
  lastCenter = center;
  return currButton;
}

void updateTgtScore(int currButton, bool hitting)
{
  if (currButton != 0)
  {
    menuOpen = true;
    lastButtonTime = millis();
    if (currButton == 1)
    {
      scoreThreshold += 1;
      if (scoreThreshold > 16)
      {
        scoreThreshold = 16;
      }
    }
    if (currButton == 2)
    {
      scoreThreshold -= 1;
      if (scoreThreshold < 8)
      {
        scoreThreshold = 8;
      }
    }
  }
  if (!hitting && millis() < lastButtonTime + menuTime)
  {
    for (int i = 0; i < NUM_LEDS; i++)
    {
      strip.setPixelColor(i, strip.Color(0, 0, 0, 0));
    }
    for (int i = 0; i < scoreThreshold - 8; i++)
    {
      strip.setPixelColor(i, strip.Color(0, 0, 30, 0));
    }
    strip.show();
  }
  else if (menuOpen)
  {
    menuOpen = false;
    strip.clear();
  }
}

void wake()
{
  // detachInterrupt(INTERRUPTPIN);
  // Serial.println("We should be awake");
  interrupted = true;
}

void getInit()
{
  initReading = getNextReading();
  targetReading = initReading - offset;
  Serial.println("Why are we doing this?");
}

void writeReading(long hTime, int reading, float rate, float score, bool hitting)
{
  if (hTime > lastSave + saveInterval)
  {
  }
}
