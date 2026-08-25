#pragma once

// ---- Wi-Fi ----
// Edit these before flashing. The buddy joins your home network at boot;
// the IP address is briefly shown on screen so you know where to point
// your phone browser (or use http://desk-buddy.local/ if your router
// supports mDNS).
#define WIFI_SSID       "YourWiFiName"
#define WIFI_PASSWORD   "YourWiFiPassword"
#define HOSTNAME        "desk-buddy"

// ---- ntfy.sh (internet messages) ----
// The buddy subscribes to this ntfy.sh topic and shows any message
// posted to it. Pick something long and unguessable — anyone who knows
// the topic can send messages to your buddy.
// Send from anywhere with:
//   curl -H "Title: Take a break" -H "Tags: coffee" \
//        -d "Grab a cup" ntfy.sh/YOUR-TOPIC-HERE
#define NTFY_TOPIC      "desk-buddy-CHANGE-ME-to-something-long-and-random"
#define NTFY_POLL_MS    3000

// ---- Behavior ----
// How long each rotating prompt stays on screen before the next one.
#define PROMPT_INTERVAL_MS  20000

// How long the bulldog "acts out" a prompt (wave, tongue out, etc.)
// before settling back to idle blinking.
#define ACTION_DURATION_MS   4000

// After a web-triggered or ntfy prompt, pause auto-rotation this long
// so your custom message actually gets read.
#define CUSTOM_HOLD_MS      25000
