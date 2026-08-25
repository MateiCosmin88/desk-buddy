// =============================================================
// Desk Buddy -- Animated French Bulldog for the CYD
//   (ESP32-2432S028R, 2.8" 240x320 ILI9341)
//
// Features:
//   * Animated French bulldog face (blink, wave, coffee, wink,
//     stretch, sleepy, tongue-out) drawn from primitives
//   * Rotating built-in wellness prompts
//   * Local web UI on http://desk-buddy.local/ for mood buttons
//     and custom messages
//   * ntfy.sh subscription -- send messages from anywhere on the
//     internet with a single curl command
//
// >>> EDIT WIFI + NTFY SETTINGS IN config.h BEFORE FLASHING <<<
// =============================================================

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <Arduino_GFX_Library.h>

#include "config.h"
#include "messages.h"
#include "bulldog.h"
#include "webui.h"
#include "ntfy.h"

// ---- CYD (ESP32-2432S028R) display pins ----
#define LCD_BL    21
#define LCD_MISO  12
#define LCD_MOSI  13
#define LCD_SCLK  14
#define LCD_CS    15
#define LCD_DC     2
#define LCD_RST   -1

static Arduino_DataBus* bus =
    new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCLK, LCD_MOSI, LCD_MISO, VSPI);
static Arduino_GFX* display =
    new Arduino_ILI9341(bus, LCD_RST, 1 /* landscape */, false);

static Bulldog     buddy(display);
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
    delay(150);
    Serial.println("Desk Buddy booting");

    pinMode(LCD_BL, OUTPUT);
    digitalWrite(LCD_BL, HIGH);

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
        Serial.println("WiFi failed -- running offline");
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

    if (!settled && now >= settleAt) {
        buddy.setMood(MOOD_IDLE);
        settled = true;
    }

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
