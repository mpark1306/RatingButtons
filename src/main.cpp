#include <Arduino.h>

// LED pins
static const uint8_t LED_PIN_RED    = 13;  // D13
static const uint8_t LED_PIN_YELLOW = 14;  // D14
static const uint8_t LED_PIN_BLUE   = 26;  // D26
static const uint8_t LED_PIN_GREEN  = 33;  // D33

// Button pins
static const uint8_t BUTTON_PIN_RED    = 12; // D12
static const uint8_t BUTTON_PIN_YELLOW = 27; // D27
static const uint8_t BUTTON_PIN_BLUE   = 25; // D25
static const uint8_t BUTTON_PIN_GREEN  = 32; // D32

// Debounce interval
static const uint16_t DEBOUNCE_MS = 20;

// State tracking for RED
uint8_t  lastStateRed    = HIGH;
uint32_t lastDebounceRed = 0;

// State tracking for YELLOW
uint8_t  lastStateYellow    = HIGH;
uint32_t lastDebounceYellow = 0;

// State tracking for BLUE
uint8_t  lastStateBlue    = HIGH;
uint32_t lastDebounceBlue = 0;

// State tracking for GREEN
uint8_t  lastStateGreen    = HIGH;
uint32_t lastDebounceGreen = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(1); }
  Serial.println("Ready. Press a button!");

  // configure all buttons as INPUT_PULLUP
  pinMode(BUTTON_PIN_RED,    INPUT_PULLUP);
  pinMode(BUTTON_PIN_YELLOW, INPUT_PULLUP);
  pinMode(BUTTON_PIN_BLUE,   INPUT_PULLUP);
  pinMode(BUTTON_PIN_GREEN,  INPUT_PULLUP);

  // configure all LEDs as OUTPUT, start LOW
  pinMode(LED_PIN_RED,    OUTPUT);
  pinMode(LED_PIN_YELLOW, OUTPUT);
  pinMode(LED_PIN_BLUE,   OUTPUT);
  pinMode(LED_PIN_GREEN,  OUTPUT);
  digitalWrite(LED_PIN_RED,    LOW);
  digitalWrite(LED_PIN_YELLOW, LOW);
  digitalWrite(LED_PIN_BLUE,   LOW);
  digitalWrite(LED_PIN_GREEN,  LOW);

  // capture initial button states
  lastStateRed    = digitalRead(BUTTON_PIN_RED);
  lastStateYellow = digitalRead(BUTTON_PIN_YELLOW);
  lastStateBlue   = digitalRead(BUTTON_PIN_BLUE);
  lastStateGreen  = digitalRead(BUTTON_PIN_GREEN);
}

// Handle red button & LED
void handleRed() {
  uint8_t reading = digitalRead(BUTTON_PIN_RED);
  uint32_t now   = millis();

  // debounce logic
  if (reading != lastStateRed) {
    lastDebounceRed = now;
  }
  if (now - lastDebounceRed > DEBOUNCE_MS && reading != lastStateRed) {
    lastStateRed = reading;
    if (reading == LOW) {
      Serial.println("Red button pressed");
    } else {
      Serial.println("Red button released");
    }
  }

  // LED follows button state
  digitalWrite(LED_PIN_RED, (reading == LOW) ? HIGH : LOW);
}

// Handle yellow button & LED
void handleYellow() {
  uint8_t reading = digitalRead(BUTTON_PIN_YELLOW);
  uint32_t now     = millis();

  if (reading != lastStateYellow) {
    lastDebounceYellow = now;
  }
  if (now - lastDebounceYellow > DEBOUNCE_MS && reading != lastStateYellow) {
    lastStateYellow = reading;
    if (reading == LOW) {
      Serial.println("Yellow button pressed");
    } else {
      Serial.println("Yellow button released");
    }
  }

  digitalWrite(LED_PIN_YELLOW, (reading == LOW) ? HIGH : LOW);
}

// Handle blue button & LED
void handleBlue() {
  uint8_t reading = digitalRead(BUTTON_PIN_BLUE);
  uint32_t now    = millis();

  if (reading != lastStateBlue) {
    lastDebounceBlue = now;
  }
  if (now - lastDebounceBlue > DEBOUNCE_MS && reading != lastStateBlue) {
    lastStateBlue = reading;
    if (reading == LOW) {
      Serial.println("Blue button pressed");
    } else {
      Serial.println("Blue button released");
    }
  }

  digitalWrite(LED_PIN_BLUE, (reading == LOW) ? HIGH : LOW);
}

// Handle green button & LED
void handleGreen() {
  uint8_t reading = digitalRead(BUTTON_PIN_GREEN);
  uint32_t now     = millis();

  if (reading != lastStateGreen) {
    lastDebounceGreen = now;
  }
  if (now - lastDebounceGreen > DEBOUNCE_MS && reading != lastStateGreen) {
    lastStateGreen = reading;
    if (reading == LOW) {
      Serial.println("Green button pressed");
    } else {
      Serial.println("Green button released");
    }
  }

  digitalWrite(LED_PIN_GREEN, (reading == LOW) ? HIGH : LOW);
}

void loop() {
  handleRed();
  handleYellow();
  handleBlue();
  handleGreen();
}
