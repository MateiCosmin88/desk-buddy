#pragma once
#include <Arduino.h>
#include "bulldog.h"

// Subscribes to a ntfy.sh topic and forwards incoming messages to
// the bulldog. Anyone with the topic URL can send from anywhere on
// the internet -- no port forwarding, no account.
//
// Publish from a phone/laptop:
//   curl -H "Title: Take a break" -H "Tags: coffee" \
//        -d "Grab a cup" ntfy.sh/YOUR-TOPIC
//
// Mapping:
//   Title header  -> line 1
//   Message body  -> line 2
//   First tag     -> mood (wave/happy/coffee/smoke/sleepy/stretch/idle)
class NtfyClient {
public:
    NtfyClient(Bulldog& b);
    void begin();
    void tick();
};
