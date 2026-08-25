#pragma once
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "messages.h"

// Animated French bulldog for the CYD (320x240 landscape) using
// Arduino_GFX. Face is drawn into an Arduino_Canvas (off-screen
// framebuffer) and pushed to the display in one transfer to avoid
// flicker.
class Bulldog {
public:
    // The passed display should already be constructed (Arduino_ILI9341
    // etc.) but not begin()'d -- Bulldog::begin() does that.
    Bulldog(Arduino_GFX* display);
    void begin();
    void showPrompt(const Prompt& p);   // auto-rotation, no hold applied
    void showCustom(const String& l1, const String& l2,
                    Mood mood, uint32_t holdMs);
    void setMood(Mood mood);
    void tick();                        // call every loop
    bool isHolding() const;             // custom prompt currently pinned
private:
    Arduino_GFX*    _display;
    Arduino_Canvas* _canvas;            // owns the drawing framebuffer

    Mood      _mood         = MOOD_IDLE;
    uint32_t  _nextBlinkAt  = 0;
    uint32_t  _blinkEndsAt  = 0;
    bool      _eyesClosed   = false;
    int       _pawFrame     = -1;
    uint32_t  _pawNextTick  = 0;
    uint32_t  _holdUntil    = 0;

    void drawText(const char* line1, const char* line2);
    void renderFace();
    void drawPaw();
    void drawRaisedPaws();
    void drawCoffeeCup();
    void drawCenteredText(const char* text, int y, uint8_t size, uint16_t color);
};
