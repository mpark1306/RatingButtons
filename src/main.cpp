#include <Arduino.h>
#include "esp_sleep.h"
#include "wifi.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

extern void setupWiFi();
extern void setupTime();
// Define a struct to hold button pin, LED pin, and button name
struct Button {
  gpio_num_t buttonPin;   // GPIO number of the button
  uint8_t ledPin;         // GPIO number of the LED
  const char* name;       // Descriptive name of the button (used in Serial output)
};

// List of button-LED pairs (GPIOs must support deep sleep wakeup)
Button buttons[] = {
  {GPIO_NUM_12, 19, "Red"},
  {GPIO_NUM_27, 21, "Yellow"},
  {GPIO_NUM_25, 22, "Blue"},
  {GPIO_NUM_32, 23, "Green"}
};

// Total number of button-LED pairs
const size_t NUM_BUTTONS = sizeof(buttons) / sizeof(buttons[0]);

// Duration for which the LED stays ON after waking up (in milliseconds)
const uint32_t LED_DURATION_MS = 7000;

// MQTT credentials and settings
const char* mqtt_server = "your-mqtt-broker.com";
const int mqtt_port = 8883; // Standard TLS port
const char* mqtt_user = "your_mqtt_username";
const char* mqtt_pass = "your_mqtt_password";
const char* mqtt_topic = "ratingbuttons/press";

// MQTT client objects
WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);

void setupMQTT() {
    espClient.setInsecure(); // For demo only! Use certificates in production.
    mqttClient.setServer(mqtt_server, mqtt_port);
    while (!mqttClient.connected()) {
        if (mqttClient.connect("ESP32Client", mqtt_user, mqtt_pass)) {
            Serial.println("MQTT connected");
        } else {
            Serial.print("MQTT connect failed, rc=");
            Serial.print(mqttClient.state());
            delay(1000);
        }
    }
}

void publishButtonPress(const char* buttonName) {
    time_t now;
    time(&now);
    char payload[128];
    snprintf(payload, sizeof(payload),
        "{\"button\":\"%s\",\"timestamp\":%ld}", buttonName, now);
    mqttClient.publish(mqtt_topic, payload);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  setupWiFi();   // Connect to WiFi
  setupTime(); 

  // Get the cause of wakeup from deep sleep
  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  Button* pressedBtn = nullptr;  // Will point to the button that caused wakeup

  // Set all button pins to INPUT_PULLUP to detect LOW signal on press
  for (size_t i = 0; i < NUM_BUTTONS; ++i) {
    pinMode(buttons[i].buttonPin, INPUT_PULLUP);
  }

  // Check if the wakeup was caused by EXT1 (external pin event)
  if (cause == ESP_SLEEP_WAKEUP_EXT1) {
    bool anyPressed = false;
    for (size_t i = 0; i < NUM_BUTTONS; ++i) {
      if (digitalRead(buttons[i].buttonPin) == LOW) {
        Serial.printf("Woke up by %s button\n", buttons[i].name);
        pinMode(buttons[i].ledPin, OUTPUT);
        digitalWrite(buttons[i].ledPin, HIGH);
        anyPressed = true;
      }
    }
    if (anyPressed) {
      delay(LED_DURATION_MS);
      // Turn off all LEDs
      for (size_t i = 0; i < NUM_BUTTONS; ++i) {
        digitalWrite(buttons[i].ledPin, LOW);
      }
    }
  }
  // Prepare wakeup mask for EXT1: any of the button pins can trigger wakeup
  uint64_t wakeup_mask = 0;
  for (size_t i = 0; i < NUM_BUTTONS; ++i) {
    wakeup_mask |= 1ULL << buttons[i].buttonPin;  // Bit-shift to set the pin's bit
  }

  // Enable EXT1 wakeup on any of the specified GPIOs being LOW
  // Note: ESP_EXT1_WAKEUP_ALL_LOW wakes up if *any* one of the specified pins goes LOW
  esp_sleep_enable_ext1_wakeup(wakeup_mask, ESP_EXT1_WAKEUP_ALL_LOW);

  // Notify user and go into deep sleep mode
  Serial.println("Sleeping now... Press any button to wake up.");
  delay(100);  // Give Serial a moment to finish printing

  // Start deep sleep
  esp_deep_sleep_start();
}

void loop() {
  // This loop is never reached because the ESP32 goes into deep sleep from setup()
}
