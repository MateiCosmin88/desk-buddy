#include "ntfy.h"
#include "config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

static Bulldog*      s_buddy       = nullptr;
static uint32_t      s_nextPollAt  = 0;

// Ring buffer of recently-seen message IDs so overlapping polls don't
// display the same message twice.
static const uint8_t ID_CACHE      = 8;
static String        s_seenIds[ID_CACHE];
static uint8_t       s_seenIdx     = 0;

static bool seen(const String& id) {
    for (uint8_t i = 0; i < ID_CACHE; i++)
        if (s_seenIds[i] == id) return true;
    return false;
}

static void markSeen(const String& id) {
    s_seenIds[s_seenIdx] = id;
    s_seenIdx = (s_seenIdx + 1) % ID_CACHE;
}

static Mood parseMood(const String& s) {
    if (s == "wave")    return MOOD_WAVE;
    if (s == "happy")   return MOOD_HAPPY;
    if (s == "coffee")  return MOOD_COFFEE;
    if (s == "smoke")   return MOOD_SMOKE;
    if (s == "sleepy")  return MOOD_SLEEPY;
    if (s == "stretch") return MOOD_STRETCH;
    return MOOD_IDLE;
}

NtfyClient::NtfyClient(Bulldog& b) { s_buddy = &b; }

void NtfyClient::begin() {
    s_nextPollAt = millis() + 500;
    Serial.printf("ntfy: subscribed to https://ntfy.sh/%s\n", NTFY_TOPIC);
}

void NtfyClient::tick() {
    if (WiFi.status() != WL_CONNECTED) return;
    uint32_t now = millis();
    if (now < s_nextPollAt) return;
    s_nextPollAt = now + NTFY_POLL_MS;

    uint32_t sinceSec = (NTFY_POLL_MS / 1000) + 2;
    String url = String("http://ntfy.sh/") + NTFY_TOPIC +
                 "/json?poll=1&since=" + sinceSec + "s";

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
        if (seen(idStr)) continue;
        markSeen(idStr);

        String l1(title);
        String l2(message);
        if (l1.length() == 0) { l1 = l2; l2 = ""; }
        if (l1.length() > 20) l1.remove(20);
        if (l2.length() > 26) l2.remove(26);

        s_buddy->showCustom(l1, l2, parseMood(tag), CUSTOM_HOLD_MS);
        Serial.printf("ntfy: '%s' / '%s' [%s]\n",
                      l1.c_str(), l2.c_str(), tag.c_str());
    }
}
