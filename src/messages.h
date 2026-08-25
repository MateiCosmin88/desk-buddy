#pragma once
#include <Arduino.h>

enum Mood {
    MOOD_IDLE,      // just blink
    MOOD_WAVE,      // paw waving
    MOOD_HAPPY,     // tongue out, big smile
    MOOD_COFFEE,    // happy + steam icon
    MOOD_SMOKE,     // cheeky wink
    MOOD_SLEEPY,    // yawn
    MOOD_STRETCH    // paws raised (stretch)
};

struct Prompt {
    const char* line1;
    const char* line2;
    Mood mood;
};

extern const Prompt PROMPTS[];
extern const size_t PROMPT_COUNT;
