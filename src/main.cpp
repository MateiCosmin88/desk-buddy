#include <Arduino.h>
#include <TFT_eSPI.h>
#include "config.h"
#include "bulldog.h"
#include "messages.h"

static TFT_eSPI tft = TFT_eSPI();
static Bulldog  buddy(tft);

static size_t   promptIdx  = 0;
static uint32_t nextPrompt = 0;
static uint32_t settleAt   = 0;
static bool     settled    = true;

void setup() {
    Serial.begin(115200);
    randomSeed(esp_random());
    buddy.begin();
    buddy.showPrompt(PROMPTS[promptIdx]);
    settleAt   = millis() + ACTION_DURATION_MS;
    settled    = false;
    nextPrompt = millis() + PROMPT_INTERVAL_MS;
}

void loop() {
    uint32_t now = millis();

    // After the action window, drop back to plain idle blinking.
    if (!settled && now >= settleAt) {
        buddy.setMood(MOOD_IDLE);
        settled = true;
    }

    // Rotate to the next prompt.
    if (now >= nextPrompt) {
        promptIdx  = (promptIdx + 1) % PROMPT_COUNT;
        buddy.showPrompt(PROMPTS[promptIdx]);
        settleAt   = now + ACTION_DURATION_MS;
        settled    = false;
        nextPrompt = now + PROMPT_INTERVAL_MS;
    }

    buddy.tick();
    delay(16);
}
