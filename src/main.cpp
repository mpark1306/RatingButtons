#include <Arduino.h>

// Button/LED debounce configuration
static const uint16_t DEBOUNCE_MS = 20;

// Define a struct for buttons
struct Button {
  uint8_t pinButton;
  uint8_t pinLED;
  uint8_t lastState;
  uint32_t lastDebounce;
  const char* name;
};

// Declare all button/LED pairs
Button buttons[] = {
  {12, 19, HIGH, 0, "Red"},    // BUTTON_PIN_RED / LED_PIN_RED
  {27, 21, HIGH, 0, "Yellow"}, // BUTTON_PIN_YELLOW / LED_PIN_YELLOW
  {25, 22, HIGH, 0, "Blue"},   // BUTTON_PIN_BLUE / LED_PIN_BLUE
  {32, 23, HIGH, 0, "Green"}   // BUTTON_PIN_GREEN / LED_PIN_GREEN
};

const size_t NUM_BUTTONS = sizeof(buttons) / sizeof(buttons[0]);

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(1); }

  Serial.println("Ready. Press a button!");

  // Setup each button and LED
  for (size_t i = 0; i < NUM_BUTTONS; ++i) {
    pinMode(buttons[i].pinButton, INPUT_PULLUP);
    pinMode(buttons[i].pinLED, OUTPUT);
    digitalWrite(buttons[i].pinLED, LOW);
    buttons[i].lastState = digitalRead(buttons[i].pinButton);
    buttons[i].lastDebounce = 0;
  }
}

void handleButton(Button &btn) {
  uint8_t reading = digitalRead(btn.pinButton);
  uint32_t now = millis();

  if (reading != btn.lastState && (now - btn.lastDebounce) > DEBOUNCE_MS) {
    btn.lastDebounce = now;
    btn.lastState = reading;

    if (reading == LOW) {
      Serial.printf("%s button pressed\n", btn.name);
    } else {
      Serial.printf("%s button released\n", btn.name);
    }
  }

  // LED mirrors the button state
  digitalWrite(btn.pinLED, (reading == LOW) ? HIGH : LOW);
}

void loop() {
  for (size_t i = 0; i < NUM_BUTTONS; ++i) {
    handleButton(buttons[i]);
  }
}
