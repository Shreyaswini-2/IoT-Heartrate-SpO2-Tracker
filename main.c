#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#define BLYNK_PRINT Serial
#include <Blynk.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "Wire.h"
#include "Adafruit_GFX.h"
#include "OakOLED.h"
#define REPORTING_PERIOD_MS 1000
OakOLED oled;
#include "DHT.h"
#define DHTPIN D5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

int a = 0, b = 0, c = 0, i = 0, sum = 0;
int avg = 0, prev = 0;

char auth[] = "iel X8iTovlAP6gXttjg54x_rJLSUueWr";
char ssid[] = "Wifi007"; // WiFi SSID
char pass[] = "12345678"; // WiFi Password

PulseOximeter pox;
int BPM, SpO2;
uint32_t tsLastReport = 0;

const unsigned char bitmap[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x18, 0x00, 0x0F, 0xE0, 0x7F, 0x00, 0x3F, 0xF9, 0xFF, 0xC0,
    // Remaining bitmap data goes here
};

void onBeatDetected() {
    Serial.println("Beat Detected!");
    oled.drawBitmap(60, 20, bitmap, 28, 28, 1);
    oled.display();
}

void setup() {
    dht.begin();
    Serial.begin(115200);
    oled.begin();
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(1);
    oled.setCursor(0, 0);
    oled.println("Initializing pulse oximeter..");
    oled.display();

    pinMode(D7, INPUT);
    pinMode(D8, OUTPUT);
    pinMode(16, OUTPUT);
    pinMode(D6, INPUT);
    pinMode(D4, OUTPUT);

    Blynk.begin(auth, ssid, pass);

    Serial.print("Initializing Pulse Oximeter..");
    if (!pox.begin()) {
        Serial.println("FAILED");
        oled.clearDisplay();
        oled.setTextSize(1);
        oled.setTextColor(1);
        oled.setCursor(0, 0);
        oled.println("FAILED");
        oled.display();
        for (;;);
    }

    Serial.println("SUCCESS");
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(1);
    oled.setCursor(0, 0);
    oled.println("SUCCESS");
    oled.display();

    pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop() {
    pox.update();
    Blynk.run();
    BPM = pox.getHeartRate();
    SpO2 = pox.getSpO2();

    Serial.print("Heart rate: ");
    Serial.print(BPM);
    Serial.print(" SpO2: ");
    Serial.print(SpO2);
    Serial.println(" %");

    Blynk.virtualWrite(V0, BPM);
    Blynk.virtualWrite(V1, SpO2);

    if (digitalRead(D7) == LOW) a = 1;
    if (digitalRead(D6) == LOW) b = 1;

    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(1);
    oled.setCursor(0, 16);
    oled.println("Heart BPM");
    oled.setCursor(0, 30);
    oled.println(BPM);
    oled.setCursor(0, 45);
    oled.println("SpO2");
    oled.setCursor(0, 60);
    oled.println(SpO2);
    oled.display();

    if (a == 1) {
        if (BPM != prev) {
            if (i <= 5) {
                i++;
                sum += BPM;
            }
            prev = BPM;
        }
    }

    if (i == 5) {
        avg = sum / 5;
        Serial.println(avg);
        oled.clearDisplay();
        oled.setTextSize(2);
        oled.setTextColor(1);
        oled.setCursor(0, 16);
        oled.println("Avg BPM");
        oled.setCursor(80, 40);
        oled.println(avg);
        oled.display();
        avg = 0;
        sum = 0;
        a = 0;
        i = 0;
        delay(1000);
        pox.begin();
    }

    if (b == 1) {
        digitalWrite(D4, HIGH);
        delay(300);

        for (;;) {
            float t = dht.readTemperature();
            delay(300);
            oled.clearDisplay();
            oled.setTextSize(2);
            oled.setTextColor(1);
            oled.setCursor(0, 16);
            oled.println("Temp");
            oled.setCursor(0, 45);
            oled.print(t);
            oled.setCursor(60, 45);
            oled.println(" *C");
            oled.display();
            b = 0;

            if (digitalRead(D6) == LOW) {
                delay(300);
                c = 1;
                break;
            }
        }

        if (c == 1) {
            c = 0;
            pox.begin();
        }
    }
}
