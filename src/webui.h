#pragma once
#include <Arduino.h>
#include "bulldog.h"

// Serves a mobile-friendly control page on the local network.
// Requires WiFi to be up before begin() is called.
class WebUI {
public:
    WebUI(Bulldog& b);
    void begin();          // starts HTTP server on port 80
    void tick();           // handles pending requests; call every loop
};
