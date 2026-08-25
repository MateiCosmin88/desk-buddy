#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "messages.h"

// Animated French bulldog for the CYD (320x240 landscape).
// Face is drawn into an off-screen sprite for flicker-free updates,
// then pushed to the display. The paw is composited into the same
// sprite so the whole face refreshes atomically.
class Bulldog {
public:
    Bulldog(TFT_eSPI& tft);
    void begin();
    void showPrompt(const Prompt& p);   // auto-rotation, no hold applied
    void showCustom(const String& line1, const String& line2,
                    Mood mood, uint32_t holdMs);   // web / ntfy input
    void setMood(Mood mood);
    void tick();                        // call every loop
    bool isHolding() const;             // true while a custom prompt is showing
private:
    TFT_eSPI&   _tft;
    TFT_eSprite _faceSpr;

    Mood        _mood         = MOOD_IDLE;
    uint32_t    _nextBlinkAt  = 0;
    uint32_t    _blinkEndsAt  = 0;
    bool        _eyesClosed   = false;
    int         _pawFrame     = -1;      // -1 = paw hidden
    uint32_t    _pawNextTick  = 0;
    uint32_t    _holdUntil    = 0;

    void drawText(const char* line1, const char* line2);
    void renderFace();
    void drawPaw();
    void drawRaisedPaws();
    void drawCoffeeCup();
};
