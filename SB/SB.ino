#include <Adafruit_LEDBackpack.h>
#include <Adafruit_GFX.h>
#include <gfxfont.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
// #include <EEPROM.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPRLS.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_NeoPixel.h>

const int chipSelect = 10;

#define LIS3DH_CS 5
#define PIN 12
#define SLEEPPIN 6
#define VBATPIN A7

#define INTERRUPTPIN 0

#define NUM_LEDS 18
#define BRIGHTNESS 18
#define NUMBRIGHTNESS 1

#define CLICKTHRESHHOLD 100
#define TIMELATENCY 200

#define RESET_PIN  -1  // set to any GPIO pin # to hard-reset on begin()
#define EOC_PIN    -1  // set to any GPIO pin to read end-of-conversion by pin
Adafruit_MPRLS mpr = Adafruit_MPRLS(RESET_PIN, EOC_PIN);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRBW + NEO_KHZ800);

byte colorCycle[7][4] = {
    {0, 40, 0, 0}, {0, 0, 40, 0}, {40, 0, 0, 0}, {40, 0, 40, 0}, {0, 40, 40, 0}, {60, 20, 0, 0}, {0, 0, 0, 40}};

Adafruit_LIS3DH lis = Adafruit_LIS3DH(LIS3DH_CS);
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

float initReading = 0;
float targetReading = 0;

const int offset = 15;
const int pullThresh = 7;

const int numReadings = 10;
float readings[numReadings]; // the readings from the analog input
int readIndex = 0;         // the index of the current reading
float total = 0;   // the running total
float average = 0; // the average

float score = 0;

float timeRatio = 0.25;
const float scoreInc = 0.015;
//score += (rate * scoreInc) * timeRatio + scoreInc

unsigned long lastActiveTime = 0;
unsigned long timeout = 7 * 1000;

unsigned long lastSleepTime = 0;
unsigned long powerdown = 10 * 1000;
bool sleeping = false;
bool recording = false;
volatile bool interrupted = false;

int saveInterval = 1 * 1000; // One second
int lastSave = 0;

String fileRoot = "data_";
int filenum = 1;
File root;
File configFile;
File currFile;

void setup()
{
  Serial.begin(9600);

  pinMode(SLEEPPIN, OUTPUT);
  digitalWrite(SLEEPPIN, HIGH);
  delay(1000);
  strip.setBrightness(BRIGHTNESS);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  alpha4.begin(0x70); // pass in the address
  alpha4.setBrightness(NUMBRIGHTNESS);
  alpha4.clear();
  alpha4.writeDigitAscii(2, 'O');
  alpha4.writeDigitAscii(3, 'K');
  alpha4.writeDisplay();

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
  else {
    strip.setPixelColor(11, strip.Color(0, 40, 0, 0));
    strip.show();
    lis.setRange(LIS3DH_RANGE_2_G); // 2, 4, 8 or 16 G!
    lis.setClick(1, CLICKTHRESHHOLD, 10,TIMELATENCY);
    Serial.println("LIS3DH found!");
  }

  // initialize all the readings to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++)
  {
    readings[thisReading] = 0;
  }

  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect))
  {
    alpha4.writeDigitAscii(0, 'N');
    alpha4.writeDigitAscii(1, 'o', "drawDots");
    alpha4.writeDigitAscii(2, 'S');
    alpha4.writeDigitAscii(3, 'D');
    alpha4.writeDisplay();
    delay(3000);
    Serial.println("Card failed, or not present");
  }
  else
  {
    Serial.println("card initialized.");
    for (int i = 0; i < 9; i++)
    {
      strip.setPixelColor(i, strip.Color(0, 10, 0, 0));
    }
    strip.show();
    delay(10);
  }
  strip.clear();
  strip.show();
  alpha4.clear();
  alpha4.writeDisplay();
  root = SD.open("/");
  configFile = SD.open("/config.cgf");
  // setConfig(configFile);
  // filenum = getFileNum(root);
  // if (EEPROM.length() == 0)
  // {
  //   EEPROM.write(0, 1);
  // }
  // filenum = EEPROM.read(0);

  for (int i = 0; i < 20; i++)
  {
    getInit();
  }
  lastSleepTime = millis();
  lastActiveTime = millis();
}

// ---------------------------------------------------------------------------------

void loop()
{
  unsigned long hTime = millis();
  if (sleeping)
  {
    // Serial.println("Sleeping");
    if (interrupted){
      detachInterrupt(INTERRUPTPIN);
      digitalWrite(SLEEPPIN, HIGH);
      lastSleepTime = millis();
      lastActiveTime = millis();
      Serial.println("Starting Bar Display");
      strip.setBrightness(BRIGHTNESS);
      strip.begin();
      Serial.println("Starting AlphaNum");
      alpha4.begin(0x70); // pass in the address
      alpha4.setBrightness(NUMBRIGHTNESS);
      alpha4.clear();
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

      // initialize all the readings to 0:
      for (int thisReading = 0; thisReading < numReadings; thisReading++)
      {
        readings[thisReading] = 0;
      }

      // see if the card is present and can be initialized:
      if (!SD.begin(chipSelect))
      {
        alpha4.writeDigitAscii(0, 'N');
        alpha4.writeDigitAscii(1, 'o', "drawDots");
        alpha4.writeDigitAscii(2, 'S');
        alpha4.writeDigitAscii(3, 'D');
        alpha4.writeDisplay();
      }
      else
      {
        Serial.println("card initialized.");
      }
      strip.clear();
      strip.show();
      root = SD.open("/");
      configFile = SD.open("/config.cgf");
      checkBattery();

      for (int i = 0; i < 20; i++)
      {
      getInit();
      }
      hTime = millis();
      interrupted = false;
      sleeping = false;
    }
  }
  else
  {
    int reading = getNextReading();
    bool hitting = checkHitting(reading);
    updatePressureBar(reading);
    updateProgressBar(score);
    updateTextDisplay(score);
    //  Serial.println(initReading);
    //  Serial.println(targetReading);
    strip.show();
    alpha4.writeDisplay();
    float rate = getRate(reading);
    if (hitting)
    {
      Serial.println(score);
      score = updateScore(rate);
      lastActiveTime = hTime;
      lastSleepTime = hTime;
      recording = true;
      if (!SD.exists(fileRoot + filenum + ".csv"))
      {
        currFile = SD.open(fileRoot + filenum + ".csv", FILE_WRITE);
        String dataString = "initReading,targetReading,offset,pullThresh,timeRatio,scoreInc";
        currFile.println(dataString);
        dataString = String(initReading) + "," + targetReading + "," + offset + "," + pullThresh + "," + timeRatio + "," + scoreInc;
        currFile.println(dataString);
        dataString = "millis,reading,rate,score,hitting";
        currFile.println(dataString);
        // print to the serial port too:
        Serial.println(dataString);
        writeReading(hTime, reading, rate, score, hitting, currFile);
      }
      else
      {
        writeReading(hTime, reading, rate, score, hitting, currFile);
      }
    }
    else
    {
      if (checkScoreTimeout(hTime))
      {
        if (recording)
        {
          currFile.close();
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
        alpha4.writeDigitRaw(0, 0x0);
        alpha4.writeDigitRaw(1, 0x0);
        alpha4.writeDigitRaw(2, 0x0);
        alpha4.writeDigitRaw(3, 0x0);
        alpha4.writeDisplay();
        digitalWrite(SLEEPPIN, LOW);
        sleeping = true;
        interrupted = false;
        attachInterrupt(digitalPinToInterrupt(INTERRUPTPIN), wake, RISING);
      }
      else
      {
        writeReading(hTime, reading, rate, score, hitting, currFile);
      }
    }
  }
}

// ---------------------------------------------------------------------------------

int getNextReading()
{
  total = total - readings[readIndex];
  readings[readIndex] = mpr.readPressure();
  Serial.println(mpr.readPressure());
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
  int pos = int(off / (100));
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
  rate = rate / 150.0;
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
  return score += (rate * scoreInc) * timeRatio + scoreInc;
}

void updateProgressBar(float score)
{
  int barTop = int(score) % 9;
  strip.setPixelColor(barTop, strip.Color(colorCycle[int(score / 9)][0], colorCycle[int(score / 9)][1], colorCycle[int(score / 9)][2], colorCycle[int(score / 9)][3]));
}

void updateTextDisplay(float score)
{
  String stringScore = String(int(score * 10));
  if (stringScore.toInt() < 10)
  {
    stringScore = "0" + stringScore;
  }
  for (int j = 0; j <= (4 - stringScore.length()); j++)
  {
    String temp = "0";
    temp += stringScore;
    stringScore = temp;
  }
  for (int i = 0; i < 4; i++)
  {
    alpha4.writeDigitAscii(0, stringScore[0]);
    alpha4.writeDigitAscii(1, stringScore[1]);
    alpha4.writeDigitAscii(2, stringScore[2], "drawDots");
    alpha4.writeDigitAscii(3, stringScore[3]);
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

bool checkScoreTimeout(unsigned long milli)
{
  if (milli > lastActiveTime + timeout)
  {
    lastActiveTime = milli;
    return true;
  }
  return false;
}

bool checkSleepTimeout(unsigned long milli)
{
  if (milli > lastSleepTime + powerdown)
  {
    return true;
  }
  return false;
}

int checkBattery()
{
  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2; // we divided by 2, so multiply back
  measuredvbat *= 3.3; // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
  float battBars = measuredvbat - 3.3;
  battBars *= 10;
  Serial.print("VBat: " ); Serial.println(measuredvbat);
  int red = (3 - battBars) * 10;
  if (red < 0) {
    red = 0;
  }
  int green = (battBars - 4) * 10;
  if (green > 30){
    green = 30;
  }
  else if (green < 0){
    green = 0;
  }
  uint32_t batteryColor = strip.Color(red, green, 0, 0);
  alpha4.writeDigitAscii(0, 'B');
  alpha4.writeDigitAscii(1, 'a');
  alpha4.writeDigitAscii(2, 't');
  alpha4.writeDigitAscii(3, 't');
  alpha4.writeDisplay();
  for (int i = 0; i<battBars; i++){
    strip.setPixelColor(i, batteryColor);
  }
  strip.show();
  delay(2000);
  strip.clear();

}

void wake()
{
  detachInterrupt(INTERRUPTPIN);
  Serial.println("We should be awake");
  interrupted = true;
}

void getInit()
{
    initReading = getNextReading();
    targetReading = initReading - offset;

}

void writeReading(long hTime, int reading, float rate, float score, bool hitting, File currFile)
{
  if (hTime > lastSave + saveInterval)
  {
    // if the file is available, write to it:
    if (currFile)
    {
      currFile.flush();
      String dataString = String(hTime) + "," + reading + "," + rate + "," + score + "," + hitting;
      currFile.println(dataString);
      // print to the serial port too:
      Serial.println(dataString);
    }
    // if the file isn't open, pop up an error:
  }
}

// void setConfig(File configFile)
// {
// }

// int getFileNum(File root)
// {
//   int numFiles = 0;
//   while (true)
//   {
//     File entry = root.openNextFile();
//     if (!entry)
//     {
//       // no more files
//       break;
//     }
//     else
//     {
//       String file_name = entry.name();

//       if (file_name.indexOf('~') != 0)
//       {
//         numFiles++;
//       }
//     }

//     Serial.println(numFiles);
//   }
//   return (numFiles);
// }
