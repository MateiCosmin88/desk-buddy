#include "messages.h"

const Prompt PROMPTS[] = {
    { "Hey there!",         "Good to see you",      MOOD_WAVE    },
    { "Take a break",       "You've earned it",     MOOD_SLEEPY  },
    { "Coffee time",        "Go grab a cup",        MOOD_COFFEE  },
    { "Stretch it out",     "Stand up, wiggle",     MOOD_STRETCH },
    { "Drink water",        "Hydrate yourself",     MOOD_HAPPY   },
    { "Smoke break?",       "Step outside a min",   MOOD_SMOKE   },
    { "Look far away",      "Rest those eyes",      MOOD_SLEEPY  },
    { "Deep breath",        "In... and out",        MOOD_HAPPY   },
    { "Snack o'clock",      "Fuel up, human",       MOOD_HAPPY   },
    { "Posture check",      "Sit up straight!",     MOOD_STRETCH },
    { "You're doing great", "Keep it up",           MOOD_WAVE    },
    { "Quick walk?",        "5 minutes will do",    MOOD_STRETCH },
};

const size_t PROMPT_COUNT = sizeof(PROMPTS) / sizeof(PROMPTS[0]);
