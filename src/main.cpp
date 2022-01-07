#include <Arduino.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <Wire.h>
#include <FlowMeter.h>

const int DC_PIN = 22;
const int CS_PIN = 21;
const int BL_PIN = 33;


FlowMeter *Meter;
long period = 1000;
long lastTime = 0;
int step = 0;
bool animate = false;

Adafruit_ST7789 tft = Adafruit_ST7789(CS_PIN, DC_PIN, -1);

void MeterISR() {
    // let our flow meter count the pulses
    Meter->count();
}

void animation(int step) {
    // draw a fun water pipe
    const int mid_y = tft.height() / 2;

    for (int i = 0; i < tft.width(); i++) {
        int step_i = i - ((step - 1) * 2);
        float y_raw = sin(float(float(step_i) / 10));
        int y = y_raw * 10;

        for (int u = 0; u < 10; u++) {
            tft.drawPixel(i, y + mid_y + u, ST77XX_BLACK);
        }
    }

    for (int i = 0; i < tft.width(); i++) {
        int step_i = i - (step * 2);
        float y_raw = sin(float(float(step_i) / 10));
        int y = y_raw * 10;

        for (int u = 0; u < 10; u++) {
            tft.drawPixel(i, y + mid_y + u, ST77XX_BLUE);
        }
    }
}

void setup(void) {
    Serial.begin(9600);

    // Backlight control
    pinMode(BL_PIN, OUTPUT);
    digitalWrite(BL_PIN, HIGH);

    tft.init(240, 240);
    tft.setRotation(2);
    tft.fillScreen(ST77XX_BLACK);

    Meter = new FlowMeter(digitalPinToInterrupt(35), UncalibratedSensor, MeterISR, RISING);

    Serial.println("Initialized");

    delay(500);
}

void loop() {
    long currentTime = millis();
    long duration = currentTime - lastTime;
    if (duration >= period) {
        Meter->tick(duration);

        tft.fillScreen(ST77XX_BLACK);
        tft.setTextColor(ST77XX_GREEN);

        tft.setCursor(0, 0);
        tft.setTextSize(2);
        tft.print("Consumed ");

        tft.setCursor(0, 20);
        tft.setTextSize(10);
        tft.print(Meter->getTotalVolume());

        tft.setCursor(0, 100);
        tft.setTextSize(2);
        tft.print(" liters");


        if (Meter->getCurrentFlowrate() > 0.1) {
            digitalWrite(BL_PIN, HIGH);
            animate = true;
        } else {
            digitalWrite(BL_PIN, LOW);
            animate = false;
        }

        lastTime = currentTime;
    }

    if (animate) {
        animation(step);

        step++;
        // water pipe has 30 animation steps
        if (step > 30) {
            step = 0;
        }
    }
}