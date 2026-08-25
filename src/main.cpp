#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <TFT_eSPI.h>
#include "config.h"
#include "bulldog.h"
#include "messages.h"
#include "webui.h"
#include "ntfy.h"

static TFT_eSPI    tft = TFT_eSPI();
static Bulldog     buddy(tft);
static WebUI       webui(buddy);
static NtfyClient  ntfy(buddy);

static size_t   promptIdx  = 0;
static uint32_t nextPrompt = 0;
static uint32_t settleAt   = 0;
static bool     settled    = true;

static bool connectWiFi() {
    Serial.printf("Connecting to WiFi '%s'", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(HOSTNAME);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    return WiFi.status() == WL_CONNECTED;
}

void setup() {
    Serial.begin(115200);
    randomSeed(esp_random());
    buddy.begin();
    buddy.showPrompt(PROMPTS[promptIdx]);
    settleAt   = millis() + ACTION_DURATION_MS;
    settled    = false;
    nextPrompt = millis() + PROMPT_INTERVAL_MS;

    if (connectWiFi()) {
        String ip = WiFi.localIP().toString();
        Serial.print("IP: "); Serial.println(ip);
        if (MDNS.begin(HOSTNAME)) {
            Serial.printf("mDNS: http://%s.local/\n", HOSTNAME);
        }
        webui.begin();
        ntfy.begin();
        buddy.showCustom(String("Web UI:"), ip, MOOD_WAVE, 8000);
        settleAt   = millis() + 8000;
        settled    = false;
        nextPrompt = millis() + 12000;
    } else {
        Serial.println("WiFi failed — running offline");
        buddy.showCustom(String("No WiFi"), String("Offline mode"),
                         MOOD_SLEEPY, 5000);
        settleAt   = millis() + 5000;
        settled    = false;
        nextPrompt = millis() + 8000;
    }
}

void loop() {
    uint32_t now = millis();
    webui.tick();
    ntfy.tick();

    // After the action window, drop back to plain idle blinking.
    if (!settled && now >= settleAt) {
        buddy.setMood(MOOD_IDLE);
        settled = true;
    }

    // Skip auto-rotation while a web-triggered / ntfy prompt is holding.
    if (!buddy.isHolding() && now >= nextPrompt) {
        promptIdx  = (promptIdx + 1) % PROMPT_COUNT;
        buddy.showPrompt(PROMPTS[promptIdx]);
        settleAt   = now + ACTION_DURATION_MS;
        settled    = false;
        nextPrompt = now + PROMPT_INTERVAL_MS;
    }

    buddy.tick();
    delay(8);
}
