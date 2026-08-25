# Desk Buddy — Animated French Bulldog

An ESP32-based desk companion that runs on the **CYD (Cheap Yellow Display,
ESP32-2432S028R)** — a 2.8" 240×320 ILI9341 touchscreen. It shows an
animated French bulldog that blinks, waves, sips coffee and rotates through
short wellness prompts (*"Take a break"*, *"Coffee time"*, *"Drink water"*,
*"Stretch it out"*), plus:

- **Local web UI** on your home network for mood buttons and custom
  messages
- **Internet messages** via ntfy.sh — send from your phone or any
  laptop with a single `curl` command, from anywhere in the world

## Hardware

- ESP32-2432S028R (CYD, 2.8" version)
- Micro-USB cable for power + flashing

## First-time setup

1. Install [PlatformIO](https://platformio.org/) (VS Code extension is
   easiest).
2. Clone this repo and open the folder in VS Code.
3. Edit `src/config.h`:
   - `WIFI_SSID` / `WIFI_PASSWORD` — your home network
   - `NTFY_TOPIC` — a **long, unguessable** string, e.g.
     `desk-buddy-42f9c8-abcxyz`. Anyone with this topic name can send
     messages to your buddy.
4. Plug in the CYD via USB.
5. In PlatformIO: **Upload** (or run `pio run --target upload` in a
   terminal).
6. Open the serial monitor at 115200 baud — the buddy prints its IP
   address on connect and also shows it on screen for a few seconds.

## Local web UI

Once online, open on your phone (same WiFi):

- `http://desk-buddy.local/` (if your router supports mDNS)
- or `http://<the-IP-shown-on-screen>/`

You get:

- A form to send a custom two-line message with a chosen mood
- A grid of quick-mood buttons (wave, coffee, wink, sleepy, stretch…)

## Sending messages over the internet (ntfy.sh)

The buddy polls the free public [ntfy.sh](https://ntfy.sh) service and
displays anything posted to your topic — no port forwarding, no account.

Send from anywhere:

```bash
curl -H "Title: Take a break" -H "Tags: coffee" \
     -d "Grab a cup" ntfy.sh/YOUR-TOPIC
```

Header mapping on the buddy:

| ntfy field         | Shown as                                    |
|--------------------|---------------------------------------------|
| `Title:` header    | Line 1 (big text)                           |
| Body               | Line 2 (small text)                         |
| First `Tags:` item | Mood — `wave`, `happy`, `coffee`, `smoke`, `sleepy`, `stretch`, `idle` |

From iOS you can install the [ntfy app](https://ntfy.sh/app) and use
Shortcuts to POST to your topic, or just bookmark the topic's web page
and type messages in the browser.

## Customising

- **Prompts**: edit `src/messages.cpp` — the rotating list. Each entry
  is a `{line1, line2, mood}` tuple.
- **Timing**: `src/config.h`
  - `PROMPT_INTERVAL_MS` — how long each auto-rotated prompt stays up
  - `ACTION_DURATION_MS` — how long the bulldog acts out a mood
  - `CUSTOM_HOLD_MS` — how long a web / ntfy message pins the display
    before auto-rotation resumes
- **Colours**: fur, snout, background and pink tones are `#define`
  macros at the top of `src/bulldog.cpp` — tweak `FUR` and `FUR_DARK`
  for a grey bulldog.

## Moods

| Mood       | Animation                                            |
|------------|------------------------------------------------------|
| `idle`     | Random blinks                                        |
| `wave`     | Paw waves up and down beside the head                |
| `happy`    | Tongue peeks out                                     |
| `coffee`   | Tongue out + steaming coffee cup in the corner      |
| `stretch`  | Two paws raised above the head                       |
| `sleepy`   | Half-lidded eyes + open yawning mouth                |
| `smoke`    | Cheeky wink                                          |

## File layout

```
src/
├── main.cpp        setup + auto-rotation loop
├── bulldog.h/.cpp  drawn bulldog + moods (TFT_eSPI sprite)
├── messages.h/.cpp rotating prompt list
├── webui.h/.cpp    local WebServer + HTML control page
├── ntfy.h/.cpp     ntfy.sh polling client
└── config.h        WiFi + ntfy topic + timings
```
