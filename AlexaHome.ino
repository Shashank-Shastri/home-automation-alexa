#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "fauxmoESP.h"
#include "Credentials.h"
#include "Hall_1.h" //Include the file depending on the room you use

fauxmoESP fauxmo;

// -----------------------------------------------------------------------------

#define SERIAL_BAUDRATE      115200

#define FAN_PIN              D0
#define LIGHT_ONE_PIN        D1
#define LIGHT_TWO_PIN        D2

#define FAN_BUTTON           D5
#define LIGHT_ONE_BUTTON     D6
#define LIGHT_TWO_BUTTON     D7

bool fanState                = false;
bool lightOneState           = false;
bool lightTwoState           = false;

bool fanButtonState          = false;
bool lightOneButtonState     = false;
bool lightTwoButtonState     = false;

bool fanLastButtonState      = false;
bool lightOneLastButtonState = false;
bool lightTwoLastButtonState = false;

// -----------------------------------------------------------------------------

void handleAlexa(unsigned char device_id, const char * device_name, bool state, unsigned char value) {
        
        Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);

        if (strcmp(device_name, FAN)==0) {
            if(state != fanState) {
                fanState = state;
                digitalWrite(FAN_PIN, !fanState ? HIGH : LOW);
            }
        } else if (strcmp(device_name, LIGHT_ONE)==0) {
            if(state != lightOneState) {
                lightOneState = state;
                digitalWrite(LIGHT_ONE_PIN, !lightOneState ? HIGH : LOW);
            }
        } else if (strcmp(device_name, LIGHT_TWO)==0) {
            if(state != lightTwoState) {
                lightTwoState = state;
                digitalWrite(LIGHT_TWO_PIN, !lightTwoState ? HIGH : LOW);
            }
        }

}
    
void setupFauxmo () {
    fauxmo.setPort(80); // This is required for gen3 devices
    fauxmo.enable(true);
    // Add virtual devices
    fauxmo.addDevice(FAN);
    fauxmo.addDevice(LIGHT_ONE);
    fauxmo.addDevice(LIGHT_TWO);
}

void setupOTA() {
    ArduinoOTA.setHostname(ROOM);
    ArduinoOTA.begin();
}

void setupWifi () {
    WiFi.mode(WIFI_STA);
    Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.println("Connection Failed! Rebooting...");
      delay(5000);
      ESP.restart();
    }
    Serial.println();
    Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());

}

void setup() {

    // Init serial port and clean garbage
    Serial.begin(SERIAL_BAUDRATE);
    Serial.println();
    Serial.println();
    Serial.println("Booting");

    // LEDs
    pinMode(FAN_PIN, OUTPUT);
    pinMode(LIGHT_ONE_PIN, OUTPUT);
    pinMode(LIGHT_TWO_PIN, OUTPUT);
    
    pinMode(FAN_BUTTON, INPUT_PULLUP);
    pinMode(LIGHT_ONE_BUTTON, INPUT_PULLUP);
    pinMode(LIGHT_TWO_BUTTON, INPUT_PULLUP);
    
    digitalWrite(FAN_PIN, HIGH);
    digitalWrite(LIGHT_ONE_PIN, HIGH);
    digitalWrite(LIGHT_TWO_PIN, HIGH);

    // Wifi
    setupWifi();
    setupOTA();
    setupFauxmo();
    
    fauxmo.onSetState(handleAlexa);
}

void loop() {
    
    ArduinoOTA.handle();
    fauxmo.handle();
    fanButtonState      = !digitalRead(FAN_BUTTON);
    lightOneButtonState = !digitalRead(LIGHT_ONE_BUTTON);
    lightTwoButtonState = !digitalRead(LIGHT_TWO_BUTTON);
    
    if (fanButtonState != fanLastButtonState && fanState != fanButtonState) {
        // if the state has changed, increment the counter
        // Delay a little bit to avoid bouncing
        fanState = fanButtonState;
        digitalWrite(FAN_PIN, !fanButtonState);
        fauxmo.setState(FAN, fanState, fanState ? 255 : 0);
        delay(50);
    }
    if (lightOneButtonState != lightOneLastButtonState && lightOneState != lightOneButtonState) {
        // if the state has changed, increment the counter
        // Delay a little bit to avoid bouncing
        lightOneState = lightOneButtonState;
        digitalWrite(LIGHT_ONE_PIN, !lightOneState);
        fauxmo.setState(LIGHT_ONE, lightOneState, lightOneState ? 255 : 0);
        delay(50);
    }
    if (lightTwoButtonState != lightTwoLastButtonState && lightTwoState != lightTwoButtonState) {
        // if the state has changed, increment the counter
        // Delay a little bit to avoid bouncing
        lightTwoState = lightTwoButtonState;
        digitalWrite(LIGHT_TWO_PIN, !lightTwoState);
        fauxmo.setState(LIGHT_TWO, lightTwoState, lightTwoState ? 255 : 0);
        delay(50);
    }
    // save the current state as the last state,
    //for next time through the loop
    fanLastButtonState      = fanButtonState;
    lightOneLastButtonState = lightOneButtonState;
    lightTwoLastButtonState = lightTwoButtonState;
}
