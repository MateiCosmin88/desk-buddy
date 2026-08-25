#pragma once
#include <Arduino.h>
#include "bulldog.h"

// Serves a mobile-friendly control page on the local network.
// Requires WiFi to be up before begin() is called.
class WebUI {
public:
    WebUI(Bulldog& b);
    void begin();
    void tick();
};
