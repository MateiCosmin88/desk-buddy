// =============================================================
// Desk Buddy -- Animated French Bulldog for the CYD
//   (ESP32-2432S028R, 2.8" 240x320 ILI9341)
//
// Features: animated bulldog face (blink, wave, coffee, wink,
// stretch, sleepy, tongue-out), rotating built-in wellness
// prompts, local web UI on http://desk-buddy.local/, and
// ntfy.sh subscription so you can push messages from anywhere
// on the internet with a single curl command.
//
// Libraries to install via Arduino IDE Library Manager:
//   * GFX Library for Arduino   (moononournation)
//   * ArduinoJson               (Benoit Blanchon)
//
// Board: ESP32 Dev Module
// Partition Scheme: Huge APP (3MB No OTA / 1MB SPIFFS)
//
// >>> EDIT the three defines under "EDIT THESE" below <<<
// =============================================================

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Arduino_GFX_Library.h>

// ===================== EDIT THESE =====================
#define WIFI_SSID       "YourWiFiName"
#define WIFI_PASSWORD   "YourWiFiPassword"
#define NTFY_TOPIC      "desk-buddy-CHANGE-ME-to-something-long-and-random"
// ======================================================

#define HOSTNAME            "desk-buddy"
#define NTFY_POLL_MS        3000
#define PROMPT_INTERVAL_MS  20000
#define ACTION_DURATION_MS   4000
#define CUSTOM_HOLD_MS      25000

// ---- CYD display pins ----
#define LCD_BL   21
#define LCD_MISO 12
#define LCD_MOSI 13
#define LCD_SCLK 14
#define LCD_CS   15
#define LCD_DC    2
#define LCD_RST  -1

// ---- Display + off-screen canvas ----
Arduino_DataBus* bus     = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCLK, LCD_MOSI, LCD_MISO, VSPI);
Arduino_GFX*     display = new Arduino_ILI9341(bus, LCD_RST, 1, false);
Arduino_Canvas*  gfx     = new Arduino_Canvas(320, 240, display);

WebServer server(80);

// ---- Moods + prompt list ----
enum Mood { MOOD_IDLE, MOOD_WAVE, MOOD_HAPPY, MOOD_COFFEE, MOOD_SMOKE, MOOD_SLEEPY, MOOD_STRETCH };

struct Prompt { const char* line1; const char* line2; Mood mood; };

const Prompt PROMPTS[] = {
    { "Hey there!",         "Good to see you",     MOOD_WAVE    },
    { "Take a break",       "You've earned it",    MOOD_SLEEPY  },
    { "Coffee time",        "Go grab a cup",       MOOD_COFFEE  },
    { "Stretch it out",     "Stand up, wiggle",    MOOD_STRETCH },
    { "Drink water",        "Hydrate yourself",    MOOD_HAPPY   },
    { "Smoke break?",       "Step outside a min",  MOOD_SMOKE   },
    { "Look far away",      "Rest those eyes",     MOOD_SLEEPY  },
    { "Deep breath",        "In... and out",       MOOD_HAPPY   },
    { "Snack o'clock",      "Fuel up, human",      MOOD_HAPPY   },
    { "Posture check",      "Sit up straight!",    MOOD_STRETCH },
    { "You're doing great", "Keep it up",          MOOD_WAVE    },
    { "Quick walk?",        "5 minutes will do",   MOOD_STRETCH },
};
const size_t PROMPT_COUNT = sizeof(PROMPTS) / sizeof(PROMPTS[0]);

// ---- Colours ----
#define BG        RGB565(15,  25,  50)
#define FUR       RGB565(210, 180, 140)
#define FUR_DARK  RGB565(160, 130,  95)
#define SNOUT     RGB565(240, 220, 185)
#define PINK      RGB565(255, 130, 170)
#define PINK_DK   RGB565(200,  80, 130)
#define BROWN     RGB565(90,   55,  25)
#define WHITE     0xFFFF
#define BLACK     0x0000
#define TEXT_L2   RGB565(180, 200, 230)

#define SCR_W   320
#define SCR_H   240
#define TEXT_Y  180
#define TEXT_H   60

// ---- Bulldog state ----
Mood     currentMood = MOOD_IDLE;
uint32_t nextBlinkAt = 0;
uint32_t blinkEndsAt = 0;
bool     eyesClosed  = false;
int      pawFrame    = -1;
uint32_t pawNextTick = 0;
uint32_t holdUntil   = 0;

// ---- Rotation timers ----
size_t   promptIdx  = 0;
uint32_t nextPrompt = 0;
uint32_t settleAt   = 0;
bool     settled    = true;
uint32_t nextPollAt = 0;

// ---- Ntfy dedupe ring ----
const uint8_t ID_CACHE = 8;
String   seenIds[ID_CACHE];
uint8_t  seenIdx = 0;

// ======================== Helpers ========================
Mood parseMood(const String& s) {
    if (s == "wave")    return MOOD_WAVE;
    if (s == "happy")   return MOOD_HAPPY;
    if (s == "coffee")  return MOOD_COFFEE;
    if (s == "smoke")   return MOOD_SMOKE;
    if (s == "sleepy")  return MOOD_SLEEPY;
    if (s == "stretch") return MOOD_STRETCH;
    return MOOD_IDLE;
}

void thickLine(int x0, int y0, int x1, int y1, int w, uint16_t color) {
    for (int i = -(w / 2); i <= (w / 2); i++) {
        gfx->drawLine(x0 + i, y0, x1 + i, y1, color);
        gfx->drawLine(x0, y0 + i, x1, y1 + i, color);
    }
}

void drawCenteredText(const char* text, int y, uint8_t size, uint16_t color) {
    if (!text) return;
    gfx->setTextSize(size);
    gfx->setTextColor(color);
    int w = (int)strlen(text) * 6 * size;
    int x = (SCR_W - w) / 2;
    if (x < 0) x = 0;
    gfx->setCursor(x, y);
    gfx->print(text);
}

void drawText(const char* l1, const char* l2) {
    gfx->fillRect(0, TEXT_Y, SCR_W, TEXT_H, BG);
    drawCenteredText(l1, TEXT_Y + 6,  3, WHITE);
    drawCenteredText(l2, TEXT_Y + 38, 2, TEXT_L2);
}

// ======================== Bulldog drawing ========================
void drawPaw() {
    int ox = 30, oy = 0;
    int baseX = ox + 200;
    int baseY;
    switch (pawFrame % 4) {
        case 0: baseY = oy + 50; break;
        case 1: baseY = oy + 30; break;
        case 2: baseY = oy + 20; break;
        case 3: baseY = oy + 35; break;
        default: baseY = oy + 45;
    }
    gfx->fillRoundRect(baseX, baseY + 10, 34, 22, 8, FUR);
    gfx->fillCircle(baseX + 6,  baseY + 6, 6, FUR);
    gfx->fillCircle(baseX + 17, baseY + 2, 6, FUR);
    gfx->fillCircle(baseX + 28, baseY + 6, 6, FUR);
    gfx->fillCircle(baseX + 6,  baseY + 6, 2, PINK);
    gfx->fillCircle(baseX + 17, baseY + 2, 2, PINK);
    gfx->fillCircle(baseX + 28, baseY + 6, 2, PINK);
}

void drawRaisedPaws() {
    int ox = 30, oy = 0;
    int py = oy + 4;
    int xs[2] = { ox + 40, ox + 190 };
    for (int i = 0; i < 2; i++) {
        int px = xs[i];
        gfx->fillRoundRect(px, py + 12, 34, 22, 8, FUR);
        gfx->fillCircle(px + 6,  py + 8, 6, FUR);
        gfx->fillCircle(px + 17, py + 4, 6, FUR);
        gfx->fillCircle(px + 28, py + 8, 6, FUR);
    }
}

void drawCoffeeCup() {
    int ox = 30, oy = 0;
    int cx = ox + 200, cy = oy + 18;
    gfx->fillRoundRect(cx, cy, 34, 30, 4, WHITE);
    gfx->drawRoundRect(cx, cy, 34, 30, 4, BLACK);
    gfx->fillRoundRect(cx + 2, cy + 2, 30, 8, 2, BROWN);
    gfx->drawCircle(cx + 38, cy + 15, 7, WHITE);
    gfx->drawCircle(cx + 38, cy + 15, 6, BLACK);
    for (int i = 0; i < 3; i++) {
        int sx = cx + 6 + i * 11;
        gfx->drawLine(sx,     cy - 4,  sx + 3, cy - 10, WHITE);
        gfx->drawLine(sx + 3, cy - 10, sx,     cy - 16, WHITE);
    }
}

void renderFace() {
    gfx->fillRect(0, 0, SCR_W, TEXT_Y, BG);
    int ox = 30, oy = 0;

    // Ears (behind head)
    gfx->fillTriangle(ox+30,  oy+50, ox+55,  oy+0,  ox+92,  oy+42, FUR);
    gfx->fillTriangle(ox+48,  oy+42, ox+60,  oy+12, ox+78,  oy+40, PINK);
    gfx->fillTriangle(ox+168, oy+42, ox+205, oy+0,  ox+230, oy+50, FUR);
    gfx->fillTriangle(ox+182, oy+40, ox+200, oy+12, ox+212, oy+42, PINK);
    // Head
    gfx->fillRoundRect(ox+25, oy+35, 210, 130, 55, FUR);
    // Nose bridge wrinkle
    gfx->fillTriangle(ox+122, oy+68, ox+138, oy+68, ox+130, oy+92, FUR_DARK);
    gfx->drawFastHLine(ox+118, oy+78, 24, FUR_DARK);
    gfx->drawFastHLine(ox+120, oy+85, 20, FUR_DARK);
    // Snout
    gfx->fillRoundRect(ox+70, oy+98, 120, 58, 28, SNOUT);

    // Eyes
    bool sleepy = (currentMood == MOOD_SLEEPY);
    bool wink   = (currentMood == MOOD_SMOKE);
    if (eyesClosed) {
        gfx->fillRect(ox+74,  oy+78, 34, 3, BLACK);
        gfx->fillRect(ox+154, oy+78, 34, 3, BLACK);
    } else if (sleepy) {
        gfx->fillCircle(ox+90,  oy+78, 16, BLACK);
        gfx->fillCircle(ox+170, oy+78, 16, BLACK);
        gfx->fillRect(ox+70,  oy+62, 40, 18, FUR);
        gfx->fillRect(ox+150, oy+62, 40, 18, FUR);
        gfx->drawFastHLine(ox+74,  oy+80, 34, BLACK);
        gfx->drawFastHLine(ox+154, oy+80, 34, BLACK);
    } else {
        gfx->fillCircle(ox+90, oy+78, 16, BLACK);
        gfx->fillCircle(ox+94, oy+73, 4,  WHITE);
        if (wink) {
            gfx->fillRect(ox+154, oy+78, 34, 3, BLACK);
            gfx->drawLine(ox+186, oy+78, ox+190, oy+74, BLACK);
        } else {
            gfx->fillCircle(ox+170, oy+78, 16, BLACK);
            gfx->fillCircle(ox+174, oy+73, 4,  WHITE);
        }
    }

    // Nose
    gfx->fillEllipse(ox+130, oy+108, 22, 12, BLACK);
    gfx->fillEllipse(ox+124, oy+104, 3,  2,  WHITE);

    // Mouth
    if (currentMood == MOOD_SLEEPY) {
        gfx->fillEllipse(ox+130, oy+140, 20, 14, BLACK);
        gfx->fillEllipse(ox+130, oy+143, 12,  8, PINK);
    } else {
        gfx->fillRect(ox+129, oy+118, 3, 22, BLACK);
        thickLine(ox+130, oy+140, ox+108, oy+148, 3, BLACK);
        thickLine(ox+130, oy+140, ox+152, oy+148, 3, BLACK);
        if (currentMood == MOOD_HAPPY || currentMood == MOOD_COFFEE || currentMood == MOOD_WAVE) {
            gfx->fillRoundRect(ox+120, oy+145, 20, 12, 4, PINK);
            gfx->drawFastVLine(ox+130, oy+148, 8, PINK_DK);
        }
    }

    if (currentMood == MOOD_COFFEE)  drawCoffeeCup();
    if (currentMood == MOOD_STRETCH) drawRaisedPaws();
    if (currentMood == MOOD_WAVE && pawFrame >= 0) drawPaw();

    gfx->flush();
}

void setMood(Mood m) {
    currentMood = m;
    if (m == MOOD_WAVE) {
        pawFrame    = 0;
        pawNextTick = millis();
    } else {
        pawFrame = -1;
    }
    renderFace();
}

void showPrompt(const Prompt& p) {
    drawText(p.line1, p.line2);
    setMood(p.mood);
}

void showCustom(const String& l1, const String& l2, Mood m, uint32_t holdMs) {
    drawText(l1.c_str(), l2.c_str());
    setMood(m);
    holdUntil = millis() + holdMs;
}

bool isHolding() { return millis() < holdUntil; }

void bulldogTick() {
    uint32_t now = millis();
    bool dirty = false;
    if (currentMood != MOOD_SLEEPY && currentMood != MOOD_SMOKE) {
        if (!eyesClosed && now >= nextBlinkAt) {
            eyesClosed  = true;
            blinkEndsAt = now + 140;
            dirty = true;
        } else if (eyesClosed && now >= blinkEndsAt) {
            eyesClosed  = false;
            nextBlinkAt = now + 2500 + random(3500);
            dirty = true;
        }
    } else if (eyesClosed) { eyesClosed = false; dirty = true; }
    if (currentMood == MOOD_WAVE && now >= pawNextTick) {
        pawFrame    = (pawFrame + 1) % 4;
        pawNextTick = now + 180;
        dirty = true;
    }
    if (dirty) renderFace();
}

// ======================== Web UI ========================
const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html><html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Desk Buddy</title>
<style>
 body{font-family:-apple-system,sans-serif;background:#0f1932;color:#eef;margin:0;padding:18px;max-width:480px;margin-left:auto;margin-right:auto}
 h1{font-size:24px;margin:0 0 14px;text-align:center}
 .card{background:#1a2547;border-radius:14px;padding:16px;margin-bottom:14px}
 label{display:block;margin:8px 0 4px;font-size:13px;opacity:.65}
 input,select,button{width:100%;padding:12px;font-size:16px;border-radius:10px;border:none;box-sizing:border-box;font-family:inherit}
 input,select{background:#2b3866;color:#fff;margin-bottom:6px}
 button.primary{background:#ff82aa;color:#111;font-weight:700;cursor:pointer;margin-top:10px}
 .moods{display:grid;grid-template-columns:repeat(3,1fr);gap:8px}
 .moods button{padding:14px 6px;font-size:14px;background:#3d4d84;color:#fff;font-weight:600;border-radius:10px;cursor:pointer}
 #status{font-size:13px;opacity:.6;margin-top:12px;text-align:center;min-height:18px}
</style></head><body>
<h1>&#x1F436; Desk Buddy</h1>
<div class="card">
 <label>Line 1</label><input id="l1" maxlength="18" placeholder="Hey there!">
 <label>Line 2</label><input id="l2" maxlength="22" placeholder="You're doing great">
 <label>Mood</label>
 <select id="mood"><option value="wave">Wave</option><option value="happy">Happy</option><option value="coffee">Coffee</option><option value="smoke">Wink</option><option value="sleepy">Sleepy</option><option value="stretch">Stretch</option><option value="idle">Idle</option></select>
 <button class="primary" onclick="send()">Send to buddy</button>
</div>
<div class="card"><label>Quick moods</label>
 <div class="moods">
  <button onclick="mood('wave')">Wave</button><button onclick="mood('happy')">Happy</button><button onclick="mood('coffee')">Coffee</button>
  <button onclick="mood('smoke')">Wink</button><button onclick="mood('sleepy')">Sleepy</button><button onclick="mood('stretch')">Stretch</button>
  <button onclick="mood('idle')">Idle</button>
 </div>
</div>
<div id="status"></div>
<script>
const say=m=>{document.getElementById('status').textContent=m;clearTimeout(window._t);window._t=setTimeout(()=>document.getElementById('status').textContent='',2200)};
const send=()=>{const l1=document.getElementById('l1').value||'Hey there!';const l2=document.getElementById('l2').value||'';const m=document.getElementById('mood').value;
 fetch('/show',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'l1='+encodeURIComponent(l1)+'&l2='+encodeURIComponent(l2)+'&mood='+m}).then(r=>say(r.ok?'Sent!':'Error')).catch(()=>say('Offline'));};
const mood=m=>{fetch('/mood?m='+m,{method:'POST'}).then(r=>say(r.ok?'Mood: '+m:'Error'));};
</script></body></html>
)HTML";

void handleShow() {
    String l1 = server.hasArg("l1")   ? server.arg("l1")   : String("");
    String l2 = server.hasArg("l2")   ? server.arg("l2")   : String("");
    String m  = server.hasArg("mood") ? server.arg("mood") : String("happy");
    if (l1.length() > 20) l1.remove(20);
    if (l2.length() > 26) l2.remove(26);
    showCustom(l1, l2, parseMood(m), CUSTOM_HOLD_MS);
    server.send(200, "text/plain", "ok");
}

void handleMood() {
    String m = server.hasArg("m") ? server.arg("m") : String("idle");
    showCustom(String("Mood"), m, parseMood(m), CUSTOM_HOLD_MS);
    server.send(200, "text/plain", "ok");
}

// ======================== Ntfy client ========================
bool ntfySeen(const String& id) {
    for (uint8_t i = 0; i < ID_CACHE; i++)
        if (seenIds[i] == id) return true;
    return false;
}
void ntfyMark(const String& id) {
    seenIds[seenIdx] = id;
    seenIdx = (seenIdx + 1) % ID_CACHE;
}

void ntfyTick() {
    if (WiFi.status() != WL_CONNECTED) return;
    uint32_t now = millis();
    if (now < nextPollAt) return;
    nextPollAt = now + NTFY_POLL_MS;

    uint32_t sinceSec = (NTFY_POLL_MS / 1000) + 2;
    String url = String("http://ntfy.sh/") + NTFY_TOPIC + "/json?poll=1&since=" + sinceSec + "s";
    HTTPClient http;
    http.setTimeout(4000);
    if (!http.begin(url)) return;
    int code = http.GET();
    if (code != HTTP_CODE_OK) { http.end(); return; }
    String body = http.getString();
    http.end();

    int start = 0;
    while (start < (int)body.length()) {
        int nl  = body.indexOf('\n', start);
        int end = (nl < 0) ? body.length() : nl;
        String line = body.substring(start, end);
        line.trim();
        start = (nl < 0) ? body.length() : nl + 1;
        if (line.length() == 0) continue;

        JsonDocument doc;
        if (deserializeJson(doc, line)) continue;

        const char* event = doc["event"] | "";
        if (event[0] && strcmp(event, "message") != 0) continue;

        const char* id      = doc["id"]      | "";
        const char* title   = doc["title"]   | "";
        const char* message = doc["message"] | "";
        String tag = "";
        if (doc["tags"].is<JsonArray>() && doc["tags"].size() > 0) {
            tag = String((const char*)doc["tags"][0]);
            tag.toLowerCase();
        }
        if (id[0] == '\0') continue;
        String idStr(id);
        if (ntfySeen(idStr)) continue;
        ntfyMark(idStr);

        String l1(title);
        String l2(message);
        if (l1.length() == 0) { l1 = l2; l2 = ""; }
        if (l1.length() > 20) l1.remove(20);
        if (l2.length() > 26) l2.remove(26);
        showCustom(l1, l2, parseMood(tag), CUSTOM_HOLD_MS);
        Serial.printf("ntfy: '%s' / '%s' [%s]\n", l1.c_str(), l2.c_str(), tag.c_str());
    }
}

// ======================== WiFi ========================
bool connectWiFi() {
    Serial.printf("Connecting to WiFi '%s'", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(HOSTNAME);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    return WiFi.status() == WL_CONNECTED;
}

// ======================== setup / loop ========================
void setup() {
    Serial.begin(115200);
    delay(150);
    Serial.println("Desk Buddy booting");

    pinMode(LCD_BL, OUTPUT);
    digitalWrite(LCD_BL, HIGH);

    randomSeed(esp_random());
    gfx->begin();
    gfx->fillScreen(BG);
    gfx->flush();

    nextBlinkAt = millis() + random(2500, 5000);
    showPrompt(PROMPTS[promptIdx]);
    settleAt   = millis() + ACTION_DURATION_MS;
    settled    = false;
    nextPrompt = millis() + PROMPT_INTERVAL_MS;

    if (connectWiFi()) {
        String ip = WiFi.localIP().toString();
        Serial.print("IP: "); Serial.println(ip);
        if (MDNS.begin(HOSTNAME)) Serial.printf("mDNS: http://%s.local/\n", HOSTNAME);

        server.on("/", HTTP_GET, []() { server.send_P(200, "text/html", INDEX_HTML); });
        server.on("/show", HTTP_POST, handleShow);
        server.on("/mood", HTTP_POST, handleMood);
        server.begin();
        Serial.println("WebUI listening on port 80");
        Serial.printf("ntfy: subscribed to https://ntfy.sh/%s\n", NTFY_TOPIC);
        nextPollAt = millis() + 500;

        showCustom(String("Web UI:"), ip, MOOD_WAVE, 8000);
        settleAt   = millis() + 8000;
        settled    = false;
        nextPrompt = millis() + 12000;
    } else {
        Serial.println("WiFi failed -- running offline");
        showCustom(String("No WiFi"), String("Offline mode"), MOOD_SLEEPY, 5000);
        settleAt   = millis() + 5000;
        settled    = false;
        nextPrompt = millis() + 8000;
    }
}

void loop() {
    uint32_t now = millis();
    server.handleClient();
    ntfyTick();

    if (!settled && now >= settleAt) {
        setMood(MOOD_IDLE);
        settled = true;
    }

    if (!isHolding() && now >= nextPrompt) {
        promptIdx  = (promptIdx + 1) % PROMPT_COUNT;
        showPrompt(PROMPTS[promptIdx]);
        settleAt   = now + ACTION_DURATION_MS;
        settled    = false;
        nextPrompt = now + PROMPT_INTERVAL_MS;
    }

    bulldogTick();
    delay(8);
}
