#include <Arduino.h>
#include <EZButton.h>
#include <esp_sleep.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>

// WiFi
const char* ssid = "IoT_H3/4";
const char* password = "98806829";

// MQTT Login
const char* mqtt_server = "192.168.0.130";
const int   mqtt_port = 1883;
// MQTT credentials
const char device_name[] = "device08";
const char mqtt_password[] = "device08-password";
// MQTT topic
const char* mqtt_topic = "sensor/device08";

// Button pins
#define RED_BTN_PIN    14
#define YELLOW_BTN_PIN 27
#define BLUE_BTN_PIN   25
#define GREEN_BTN_PIN  33

// LED pins
#define RED_LED_PIN     19
#define YELLOW_LED_PIN  21
#define BLUE_LED_PIN    22
#define GREEN_LED_PIN   23

// Bitmask for deep-sleep wake
#define WAKEUP_PINS ((1ULL << RED_BTN_PIN) | \
                     (1ULL << YELLOW_BTN_PIN) | \
                     (1ULL << BLUE_BTN_PIN)  | \
                     (1ULL << GREEN_BTN_PIN))

// EZButton setup
struct ezButtonMap {
  ezButton button;
  uint8_t btnPin;
  uint8_t ledPin;
  const char* label;
};

ezButtonMap buttonMap[] = {
  { ezButton(RED_BTN_PIN),    RED_BTN_PIN,    RED_LED_PIN,    "RED" },
  { ezButton(YELLOW_BTN_PIN), YELLOW_BTN_PIN, YELLOW_LED_PIN, "YELLOW" },
  { ezButton(BLUE_BTN_PIN),   BLUE_BTN_PIN,   BLUE_LED_PIN,   "BLUE" },
  { ezButton(GREEN_BTN_PIN),  GREEN_BTN_PIN,  GREEN_LED_PIN,  "GREEN" },
};

// Track time and LED status
unsigned long wakeTime;
bool ledActive = false;
unsigned long ledTimerStart = 0;
uint8_t activeLedPin = 0;

// WiFi and MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Helper to get formatted timestamp
void getFormattedTimestamp(char* buffer, size_t len) {
  time_t now = time(nullptr);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  strftime(buffer, len, "%H:%M %d/%m/%Y", &timeinfo);
}

// Connect to WiFi
void setup_wifi() {
  delay(10);
  Serial.println("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  // Configure NTP
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP time sync");
  time_t now = time(nullptr);
  int retry = 0;
  const int retry_count = 20;
  while (now < 1000000000 && ++retry < retry_count) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println();
  if (now < 1000000000) {
    Serial.println("Failed to get NTP time.");
  } else {
    Serial.print("NTP time synced: ");
    Serial.println(now);
  }
}

// Connect to MQTT broker
void reconnect_mqtt() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect(device_name, device_name, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 2s");
      delay(2000);
    }
  }
}

// Use ext1 status (hardware-level) to see which button triggered wake
void printWakeReasonAndSendMQTT() {
  uint64_t wakeStatus = esp_sleep_get_ext1_wakeup_status();

  for (auto &entry : buttonMap) {
    if (wakeStatus & (1ULL << entry.btnPin)) {
      Serial.print("Woke up from ");
      Serial.print(entry.label);
      Serial.println(" button!");
      digitalWrite(entry.ledPin, HIGH);
      activeLedPin = entry.ledPin;
      ledActive = true;
      ledTimerStart = millis();

      // Build JSON payload with formatted timestamp
      StaticJsonDocument<128> doc;
      doc["feedback"] = entry.label;
      char timeStr[32];
      getFormattedTimestamp(timeStr, sizeof(timeStr));
      doc["timestamp"] = timeStr;

      char payload[128];
      serializeJson(doc, payload);

      if (client.connected()) {
        client.publish(mqtt_topic, payload);
        Serial.print("MQTT sent: ");
        Serial.println(payload);
      } else {
        Serial.println("MQTT not connected, message not sent.");
      }
      break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(500); // Let Serial settle

  // Configure all buttons & LEDs
  for (auto &entry : buttonMap) {
    entry.button.setDebounceTime(50);
    pinMode(entry.btnPin, INPUT_PULLUP);
    pinMode(entry.ledPin, INPUT_PULLUP);
    pinMode(entry.ledPin, OUTPUT);
    digitalWrite(entry.ledPin, LOW);
  }

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  reconnect_mqtt();

  // Check if we woke from deep sleep
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT1) {
    printWakeReasonAndSendMQTT();
  } else {
    Serial.println("Powered on or reset, not a deep-sleep wake.");
  }

  // Enable wake from any of the button pins
  esp_sleep_enable_ext1_wakeup(WAKEUP_PINS, ESP_EXT1_WAKEUP_ANY_HIGH);

  // Record start time
  wakeTime = millis();
}

void loop() {
  client.loop(); // Keep MQTT connection alive

  // Only allow a new button press if no LED is currently active
  if (!ledActive) {
    for (auto &entry : buttonMap) {
      entry.button.loop();
      if (entry.button.isPressed()) {
        digitalWrite(entry.ledPin, HIGH);
        activeLedPin = entry.ledPin;
        ledActive = true;
        ledTimerStart = millis();

        // Build JSON payload with formatted timestamp
        StaticJsonDocument<128> doc;
        doc["feedback"] = entry.label;
        char timeStr[32];
        getFormattedTimestamp(timeStr, sizeof(timeStr));
        doc["timestamp"] = timeStr;

        char payload[128];
        serializeJson(doc, payload);

        if (client.connected()) {
          client.publish(mqtt_topic, payload);
          Serial.print("MQTT sent: ");
          Serial.println(payload);
        }
        break; // Only allow one button action at a time
      }
    }
  } else {
    // Still update button state for debounce, but ignore presses
    for (auto &entry : buttonMap) {
      entry.button.loop();
    }
  }

  // After 7 seconds, turn off whichever LED was lit
  if (ledActive && (millis() - ledTimerStart >= 7000)) {
    digitalWrite(activeLedPin, LOW);
    ledActive = false;
  }

  // Go back to deep sleep after 10 seconds total
  if (millis() - wakeTime >= 10000) {
    Serial.println("Waiting for all buttons to be released...");
    // Wait for all buttons to be released
    bool anyPressed;
    do {
      anyPressed = false;
      for (auto &entry : buttonMap) {
        entry.button.loop();
        if (entry.button.getState() == HIGH) {
          anyPressed = true;
        }
      }
      delay(10); // debounce
    } while (anyPressed);

    Serial.println("Going back to deep sleep...");
    esp_deep_sleep_start();
  }
}