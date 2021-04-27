#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <BlynkSimpleEsp32_BLE.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <NeoPixelBus.h>


#define BLYNK_PRINT Serial
#define BLYNK_USE_DIRECT_CONNECT
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define POT 13
#define colorSaturation 128

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_BMP280 bmp; // I2C
BlynkTimer timer;

char auth[] = "kUZUwBvDQRdI60Dx2RbTJ3sUkaTGETEX";
int count = 0;

const int ledPin = 2;
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

const uint16_t PixelCount = 12;
const uint8_t PixelPin = 4;
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);
RgbColor red(colorSaturation, 0, 0);
RgbColor black(0, 0, 0);

void data(int _count) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(23, 4);
  display.println("BME280 and POT");
  display.drawRect(0, 0, display.width(), 15, SSD1306_WHITE);
  display.drawRect(0, 16, display.width(), 47, SSD1306_WHITE);
  display.setCursor(5, 20);
  display.println("Temp:");
  display.setCursor(45, 20);
  display.print(bmp.readTemperature());
  display.setCursor(5, 30);
  display.println("Pres:");
  display.setCursor(45, 30);
  display.print(bmp.readPressure());
  display.setCursor(5, 40);
  display.println("Alt:");
  display.setCursor(45, 40);
  display.print(bmp.readAltitude(1031.75));
  display.setCursor(5, 50);
  display.println("Pot:");
  display.setCursor(45, 50);
  display.print(analogRead(POT));
  display.setTextSize(3);
  display.setCursor(101, 30);
  display.println(_count);
  display.display();
}

void myTimerEvent()
{
  int valuePot = analogRead(POT);
  Blynk.virtualWrite(V0, valuePot);

  float valueTemp = bmp.readTemperature();
  Blynk.virtualWrite(V1, valueTemp);

  int valueAlt = bmp.readAltitude(1031.75);
  Blynk.virtualWrite(V2, valueAlt);

  int valuePres = bmp.readPressure();
  Blynk.virtualWrite(V3, valuePres);

  data(count);
}

BLYNK_WRITE(V4)
{
  int pinValue = param.asInt();
  Serial.println(pinValue);
  if (pinValue == 1 && count < 10)
  {
    data(count);
    if (count == 9)
    {
      count = -1;
    }
    count++;
  }
}

BLYNK_WRITE(V5)
{
  int dataSlider = param.asInt(); 
  int pixels = map(dataSlider, 0, 255, 0, 11);
  for (int i = 0; i <= 11; i++)
  {
    strip.SetPixelColor(i, black);
  }
  strip.Show();
  for (int i = 0; i <= pixels; i++)
  {
    strip.SetPixelColor(i, red);
  }
  strip.Show();
  ledcWrite(ledChannel, dataSlider);
  if (dataSlider == 0)
  {
    Blynk.virtualWrite(V6, 0);
  } else if (dataSlider == 255)
  {
    Blynk.virtualWrite(V6, 1);
  }
}

BLYNK_WRITE(V6)
{
  int dataButton = param.asInt(); 
  if (dataButton == 0)
  {
    ledcWrite(ledChannel, 0);  
    Blynk.virtualWrite(V5, 0);
  } else {
    ledcWrite(ledChannel, 255); 
    Blynk.virtualWrite(V5, 255); 
  }
}

void setup() {
  Serial.begin(9600);

  strip.Begin();
  strip.Show();

  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(ledPin, ledChannel);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  Serial.println("Waiting for connections...");
  Blynk.setDeviceName("miDisEjemplo");
  Blynk.begin(auth);
  timer.setInterval(100L, myTimerEvent);
}

void loop() {
  Blynk.run();
  timer.run();
}