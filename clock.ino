#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include "secrets.h"
#include <WiFi.h>
#include <Wire.h>

#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT    32
#define OLED_RESET       -1
#define SCREEN_ADDRESS 0x3C
#define BUTTON 4
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int i = 0;
const char* ssid = SSID;
const char* password = WIFI_PASSWORD;

const int httpsPort = 443;
const String url_time = "https://www.timeapi.io/api/Time/current/zone?timeZone=" + String(ZONE);
const String url_weather = "https://api.openweathermap.org/data/2.5/weather?lat=" + String(LAT) + "&lon=" + String(LON) + "&appid=" + String(APPID);

WiFiClient client;
HTTPClient http;

void setup()
{
    Serial.begin(115200);

    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println("Connecting to WiFi...");
    display.display();

    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    display.println("Connected to: ");
    display.print(ssid);
    display.display();
    delay(1500);
    display.clearDisplay();
    display.display();

    Serial.print("Connecting to ");
    Serial.println(url_time);

    http.begin(url_time);
    int httpCode = http.GET();
    StaticJsonDocument<2000> doc;
    DeserializationError error = deserializeJson(doc, http.getString());

    if (error) {
        Serial.print(F("deserializeJson Failed"));
        Serial.println(error.f_str());
        delay(2500);
        return;
    }
    Serial.print("HTTP Status Code: ");
    Serial.println(httpCode);

    int hour = doc["hour"];
    int minute = doc["minute"];
    int second = doc["seconds"];
    int day = doc["day"];
    int month = doc["month"];
    int year = doc["year"];
    http.end();

    setTime(hour, minute, second, day, month, year);
}

void loop()
{
    if (i < 60) {
        timen();
        i++;
    } else if (i < 80) {
        daten();
        i++;
    } else {
        weather();
        i = 0;
    }
}

void weather(void)
{
    Serial.print("Connecting to ");
    Serial.println(url_weather);
    display.clearDisplay();

    http.begin(url_weather);
    int httpCode = http.GET();
    StaticJsonDocument<2000> doc;
    DeserializationError error = deserializeJson(doc, http.getString());

    if (error) {
        Serial.print(F("deserializeJson Failed"));
        Serial.println(error.f_str());
        delay(2500);
        return;
    }
    Serial.print("HTTP Status Code: ");
    Serial.println(httpCode);

    String main = doc["weather"][0]["description"].as<String>();
    double temp = doc["main"]["temp"];
    int humidity = doc["main"]["humidity"];
    double wind = doc["wind"]["speed"];
    int pressure = doc["main"]["pressure"];
    
    String weather[5]; 
    weather[0] = "Main: " + main;
    weather[1] = "Temp: " + String((int)(temp - 273.15)) + "C";
    weather[2] = "Hum: " + String(humidity) + "%";
    weather[3] = "Wind: " + String(wind) + "m/s";
    weather[4] = "Pres: " + String((int)(pressure/1.33322)) + "mm";
    
    for (uint8_t i = 0; i < 5; i++) {
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(15, 15);
        display.clearDisplay();
        center(weather[i], 5, 5);
        display.display();
        delay(3000);
    }

    http.end();
}

void timen(void)
{
    display.clearDisplay();

    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(15, 15);
    digits(hour());
    display.print(F(":"));
    digits(minute());
    display.print(F(":"));
    digits(second());

    display.display();
    delay(250);
}

void daten(void)
{
    display.clearDisplay();

    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(5, 15);
    digits(day());
    display.print(F("."));
    digits(month());
    display.print(F("."));
    digits(year());

    display.display();
    delay(250);
}

void digits(int digits)
{
    if (digits < 10) {
        display.print("0");
        display.print(digits);
    }
    else
        display.print(digits);
}

void center(const String buf, int x, int y)
{
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(buf, x, y, &x1, &y1, &w, &h); 
    display.setCursor((x - w / 2) + (128 / 2), y);
    display.print(buf);
}
