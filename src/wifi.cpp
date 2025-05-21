#include <WiFi.h>
#include "time.h"

const char* WIFI_SSID = "IoT_43/4";
const char* WIFI_PASS = "98806829";
const char* NTP_SERVER = "pool.ntp.org";
const char* TIMEZONE = "CET-1CEST,M3.5.0/2,M10.5.0/3";

void setupWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
}

void setupTime() {
    configTime(0, 0, NTP_SERVER);
    setenv("TZ", TIMEZONE, 1);
    tzset();
}