#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <secrets.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

// Firebase library
#include <FirebaseClient.h>

// Pin configuration
#define ONE_WIRE_BUS 2 // Pin for the DS18B20 sensor
#define BUZZER_PIN 13  // Pin for the buzzer

// Declaring helper functions
void connectToWifi();
void saveToDatabase(float temperatureC);

// Set up the LCD
LiquidCrystal lcd(19, 23, 18, 5, 4, 15); // RS, E, D4, D5, D6, D7

// Set up the OneWire and DallasTemperature libraries
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Firebase related declarations
RealtimeDatabase Database;
WiFiClientSecure ssl;
DefaultNetwork network;
AsyncClientClass client(ssl, getNetwork(network));
FirebaseApp app;
AsyncResult result;
NoAuth noAuth;
float previousTemperature;

void setup()
{
    pinMode(BUZZER_PIN, OUTPUT); // Set buzzer pin as output
    previousTemperature = 0;     // Initialize temperature
    lcd.begin(16, 2);            // Initialize LCD
    sensors.begin();             // Start the temperature sensor
    Serial.begin(115200);        // Initialize Serial Monitor

    delay(500);  // Wait for LCD to initialize
    lcd.clear(); // Clear the LCD screen

    connectToWifi();

    // Initialize Firebase app
    Serial.println("Initializing the app...");
    initializeApp(client, app, getAuth(noAuth));
    app.getApp<RealtimeDatabase>(Database);
    Database.url(FIREBASE_DATABASE_URL);

    client.setAsyncResult(result);
    ssl.setInsecure();
}

void loop()
{
    sensors.requestTemperatures();
    float temperatureC = sensors.getTempCByIndex(0);

    Serial.print("Temperature: ");
    Serial.println(temperatureC);

    // Display temperature on LCD
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temperatureC);
    lcd.print(" C   ");

    if (temperatureC > 40)
    {
        lcd.setCursor(0, 1);
        lcd.print("Alert! Temp > 40C");
        tone(BUZZER_PIN, 1000); // 1kHz buzzer tone
        Serial.println("Temperature exceeded 40°C, turning buzzer ON!");
    }
    else
    {
        noTone(BUZZER_PIN); // Stop buzzer
        lcd.setCursor(0, 1);
        lcd.print("                "); // Clear alert message (2nd row)
    }

    if (temperatureC != previousTemperature)
    {
        saveToDatabase(temperatureC);
        previousTemperature = temperatureC;
    }

    delay(1000);
}

// Connect to WiFi
void connectToWifi()
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    lcd.setCursor(0, 0);
    lcd.print("Connecting...   ");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to Wi-Fi...");
    }
    lcd.setCursor(0, 1);
    lcd.print("WiFi Connected  ");
    Serial.println("Wi-Fi connected");
    delay(1000);
}

// Store temperature to Firebase
void saveToDatabase(float temperatureC)
{
    Serial.print("Saving temperature value... ");
    bool status = Database.set<float>(client, "/temperature", temperatureC);

    if (status)
        Serial.println("ok");
    else
    {
        int errorCode = client.lastError().code();
        String errorMsg = client.lastError().message();
        Serial.print("Error ");
        Serial.print(errorCode);
        Serial.print(": ");
        Serial.println(errorMsg);
    }
}
