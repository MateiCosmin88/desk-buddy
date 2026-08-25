# Desk Buddy — Animated French Bulldog

An Arduino IDE sketch for the **CYD (Cheap Yellow Display,
ESP32-2432S028R)** — 2.8" 240×320 ILI9341 touchscreen. It shows an
animated French bulldog that blinks, waves, sips coffee, winks and
rotates through short wellness prompts (*"Take a break"*,
*"Coffee time"*, *"Drink water"*, *"Stretch it out"*), plus:

- **Local web UI** on your home network for mood buttons and custom
  messages
- **Internet messages** via ntfy.sh — send from your phone or any
  laptop with a single `curl` command, from anywhere in the world

## Hardware

- ESP32-2432S028R (CYD, 2.8" version)
- USB cable

## Setup (Arduino IDE)

### 1. Install the ESP32 board support

- Open **File → Preferences**, add this to *Additional boards manager
  URLs*:
  ```
  https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
  ```
- **Tools → Board → Boards Manager…** → search "esp32" → install
  *esp32 by Espressif Systems*.

### 2. Install two libraries

**Sketch → Include Library → Manage Libraries…** and install:

- **GFX Library for Arduino** by *moononournation*
- **ArduinoJson** by *Benoit Blanchon*

That's it — WiFi, WebServer, HTTPClient and ESPmDNS all come with
the ESP32 board package.

### 3. Configure WiFi and ntfy topic

Open **`config.h`** in the sketch and edit these three lines near the
top — these are the only settings you have to touch:

```cpp
#define WIFI_SSID       "YourWiFiName"        // your home WiFi
#define WIFI_PASSWORD   "YourWiFiPassword"    // its password
#define NTFY_TOPIC      "desk-buddy-CHANGE-ME-to-something-long-and-random"
```

The **NTFY_TOPIC** is what lets you send messages from the internet.
Pick something long and unguessable (e.g. `desk-buddy-mateicosmin88-42f9c8`)
— anyone who knows that string can send messages to your buddy, so
treat it like a weak password.

### 4. Board settings

- **Tools → Board → ESP32 Arduino → ESP32 Dev Module**
- **Tools → Partition Scheme → Huge APP (3MB No OTA)** (needed — the
  sketch is big with WiFi + WebServer + JSON parsing)
- **Tools → Flash Size → 4MB**
- **Tools → Upload Speed → 921600**
- **Tools → Port →** the CYD's port (appears when USB is plugged in)

### 5. Upload

Click the **Upload** arrow. Open **Serial Monitor** at 115200 baud —
on boot the buddy prints its IP address and also shows it on screen
for a few seconds.

## Local web UI

Once online, open on your phone (same WiFi):

- `http://desk-buddy.local/` — if your router supports mDNS
- or `http://<the-IP-shown-on-screen>/`

You get a form for two-line custom messages plus a quick-mood button
grid.

## Sending messages over the internet (ntfy.sh)

The buddy polls the free public [ntfy.sh](https://ntfy.sh) service and
displays anything posted to your topic — no port forwarding, no
account, no signup.

Send from anywhere:

```bash
curl -H "Title: Take a break" -H "Tags: coffee" \
     -d "Grab a cup" ntfy.sh/YOUR-TOPIC
```

From iOS you can install the [ntfy app](https://ntfy.sh/app) and use
Shortcuts to POST to your topic, or just bookmark the topic's web
page and type messages in the browser.

Mapping on the buddy:

| ntfy field          | Shown as                                             |
|---------------------|------------------------------------------------------|
| `Title:` header     | Line 1 (big text)                                    |
| Body                | Line 2 (small text)                                  |
| First `Tags:` item  | Mood — `wave`, `happy`, `coffee`, `smoke`, `sleepy`, `stretch`, `idle` |

## Customising

- **Built-in rotating prompts**: edit `messages.cpp` — the `PROMPTS[]`
  list. Each entry is a `{line1, line2, mood}` tuple.
- **Timing**: `config.h`
  - `PROMPT_INTERVAL_MS` — how long each auto-rotated prompt stays up
  - `ACTION_DURATION_MS` — how long the bulldog acts out a mood
  - `CUSTOM_HOLD_MS` — how long a web / ntfy message pins the display
- **Colours**: fur, snout, background and pink tones are `#define`
  macros at the top of `bulldog.cpp` — tweak `FUR` and `FUR_DARK`
  for a grey bulldog.

## Moods

| Mood       | Animation                                            |
|------------|------------------------------------------------------|
| `idle`     | Random blinks                                        |
| `wave`     | Paw waves up and down beside the head                |
| `happy`    | Tongue peeks out                                     |
| `coffee`   | Tongue out + steaming coffee cup in the corner       |
| `stretch`  | Two paws raised above the head                       |
| `sleepy`   | Half-lidded eyes + open yawning mouth                |
| `smoke`    | Cheeky wink                                          |

## File layout

```
desk-buddy/                (sketch folder, matches the .ino name)
├── desk-buddy.ino         entry point: setup() + loop()
├── config.h               *** WiFi + ntfy topic + timings ***
├── bulldog.h / .cpp       drawn bulldog + moods (Arduino_GFX canvas)
├── messages.h / .cpp      rotating prompt list
├── webui.h / .cpp         local WebServer + HTML control page
├── ntfy.h / .cpp          ntfy.sh polling client
└── README.md              this file
```
