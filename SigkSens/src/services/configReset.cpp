#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#include <FS.h> //this needs to be first, or it all crashes and burns...
#ifdef ESP32
#include "SPIFFS.h"
#endif

#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#include "../../sigksens.h"
#include "../../config.h"

bool currentButtonState = false;
bool lastButtonState = false;
unsigned long buttonPressedTime = 0;


void setupConfigReset() {
  pinMode(RESET_CONFIG_PIN, INPUT_PULLUP);

}


void resetConfig() {
  WiFiManager wifiManager;
  Serial.println(F("Resetting Config!"));
  delay(200);
  wifiManager.resetSettings();
  SPIFFS.format();
  for (uint8_t x=0; x<50; x++) { // need delay as resetting right away 
    digitalWrite(LED_PIN, LOW);  // to cause wifi settings to not erase
    delay(20);
    digitalWrite(LED_PIN, HIGH);
    delay(20);
  }

  #ifdef ESP8266
    ESP.reset();
  #elif defined(ESP32)
    ESP.restart();
  #endif
}


void handleConfigReset() {
  if (digitalRead(RESET_CONFIG_PIN) == LOW) {
    currentButtonState = true; //pressed
  } else {
    currentButtonState = false; //not pressed
  }

  if (currentButtonState && !lastButtonState) { 
    //just pressed!
    lastButtonState = currentButtonState;
    buttonPressedTime = millis();
  }

  if (!currentButtonState && lastButtonState) {
    lastButtonState = currentButtonState;
    // just released

    if (((millis() - buttonPressedTime) > SHORT_BUTTON_PRESS_MS) && ((millis() - buttonPressedTime) <= LONG_BUTTON_PRESS_MS)) { // short press
      Serial.println(F("Short Button press!"));      
    } else if ((millis() - buttonPressedTime) > LONG_BUTTON_PRESS_MS) {
      Serial.println(F("Long Button press! Resetting!"));
      resetConfig();
    }
  }
}
