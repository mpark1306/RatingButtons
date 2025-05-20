#include <Arduino.h>
#include "esp_sleep.h"

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

void setup() {
  Serial.begin(115200);    // Start serial communication
  delay(1000);             // Give time for Serial Monitor to connect

  // Get the cause of wakeup from deep sleep
  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  Button* pressedBtn = nullptr;  // Will point to the button that caused wakeup

  // Set all button pins to INPUT_PULLUP to detect LOW signal on press
  for (size_t i = 0; i < NUM_BUTTONS; ++i) {
    pinMode(buttons[i].buttonPin, INPUT_PULLUP);
  }

  // Check if the wakeup was caused by EXT1 (external pin event)
  if (cause == ESP_SLEEP_WAKEUP_EXT1) {
    // Check which button is currently being pressed (reads LOW)
    for (size_t i = 0; i < NUM_BUTTONS; ++i) {
      if (digitalRead(buttons[i].buttonPin) == LOW) {
        pressedBtn = &buttons[i];
        break;  // Stop checking once we find the pressed button
      }
    }

    // If a pressed button was identified
    if (pressedBtn) {
      Serial.printf("Woke up by %s button\n", pressedBtn->name);

      // Turn on corresponding LED
      pinMode(pressedBtn->ledPin, OUTPUT);
      digitalWrite(pressedBtn->ledPin, HIGH);

      // Keep LED on for 7 seconds
      delay(LED_DURATION_MS);

      // Turn off the LED
      digitalWrite(pressedBtn->ledPin, LOW);
    }
  } else {
    // If it's a cold boot or wakeup reason is not EXT1
    Serial.println("Cold boot or unknown wakeup.");
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
