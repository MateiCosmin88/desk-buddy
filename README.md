# Desk Buddy — Animated French Bulldog

An ESP32-based desk companion that runs on the **CYD (Cheap Yellow Display,
ESP32-2432S028R)** — a 2.8" 240×320 ILI9341 touchscreen. It shows an
animated French bulldog that blinks, waves, sips coffee and rotates through
short wellness prompts like *"Take a break"*, *"Coffee time"*, *"Drink
water"*, *"Stretch it out"*.

No Wi-Fi required — everything runs locally on the device.

## Hardware

- ESP32-2432S028R (CYD, 2.8" version)
- Micro-USB cable for power + flashing

## Build & flash

Uses [PlatformIO](https://platformio.org/). From the repo root:

```
pio run --target upload
pio device monitor
```

The `platformio.ini` targets an `esp32dev` board with TFT_eSPI pin
configuration for the CYD baked in as build flags — no separate
`User_Setup.h` needed.

## Customising

- **Prompts**: edit `src/messages.cpp` — add, remove or reword entries in
  the `PROMPTS[]` array. Each entry pairs two short text lines with a mood
  (`MOOD_WAVE`, `MOOD_COFFEE`, `MOOD_STRETCH`, `MOOD_SLEEPY`, `MOOD_SMOKE`,
  `MOOD_HAPPY`, `MOOD_IDLE`).
- **Timing**: `src/config.h`
  - `PROMPT_INTERVAL_MS` — how long each prompt stays up before the next.
  - `ACTION_DURATION_MS` — how long the bulldog acts out the mood before
    settling back to idle blinking.
- **Colours**: fur, snout, background and pink tones are `#define` macros
  at the top of `src/bulldog.cpp` — swap in a grey bulldog by tweaking
  `FUR` and `FUR_DARK`.

## What each mood looks like

| Mood            | Animation                                        |
|-----------------|--------------------------------------------------|
| `MOOD_IDLE`     | Just blinks at random intervals                  |
| `MOOD_WAVE`     | Paw waves up and down beside the head            |
| `MOOD_HAPPY`    | Tongue peeks out                                 |
| `MOOD_COFFEE`   | Tongue out + a steaming coffee cup in the corner |
| `MOOD_STRETCH`  | Two paws raised above the head                   |
| `MOOD_SLEEPY`   | Half-lidded eyes + open yawning mouth            |
| `MOOD_SMOKE`    | Cheeky wink                                      |

## Layout

- **Face sprite**: 260×180 at the top, centred horizontally
- **Text area**: bottom 60px — line 1 in Font 4, line 2 in Font 2

Drawn entirely with TFT_eSPI primitives (no image assets), rendered into an
off-screen `TFT_eSprite` and pushed to the display in one transfer for
flicker-free animation.
