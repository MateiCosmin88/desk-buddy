#include "bulldog.h"

#define SCR_W    320
#define SCR_H    240
#define FACE_X   30
#define FACE_Y   0
#define FACE_W   260
#define FACE_H   180
#define TEXT_Y   (FACE_Y + FACE_H)
#define TEXT_H   (SCR_H - TEXT_Y)

#define BG        TFT_eSPI::color565(15,  25,  50)
#define FUR       TFT_eSPI::color565(210, 180, 140)
#define FUR_DARK  TFT_eSPI::color565(160, 130,  95)
#define SNOUT     TFT_eSPI::color565(240, 220, 185)
#define PINK      TFT_eSPI::color565(255, 130, 170)
#define PINK_DK   TFT_eSPI::color565(200,  80, 130)
#define BROWN     TFT_eSPI::color565(90,   55,  25)

Bulldog::Bulldog(TFT_eSPI& tft) : _tft(tft), _faceSpr(&tft) {}

void Bulldog::begin() {
    _tft.init();
    _tft.setRotation(1);
    _tft.fillScreen(BG);
    _faceSpr.setColorDepth(16);
    _faceSpr.createSprite(FACE_W, FACE_H);
    _nextBlinkAt = millis() + random(2500, 5000);
    renderFace();
}

void Bulldog::showPrompt(const Prompt& p) {
    _tft.fillRect(0, TEXT_Y, SCR_W, TEXT_H, BG);
    _tft.setTextDatum(TC_DATUM);
    _tft.setTextColor(TFT_WHITE, BG);
    _tft.setTextFont(4);
    _tft.drawString(p.line1, SCR_W / 2, TEXT_Y + 4);
    _tft.setTextFont(2);
    _tft.setTextColor(TFT_eSPI::color565(180, 200, 230), BG);
    _tft.drawString(p.line2, SCR_W / 2, TEXT_Y + 34);
    setMood(p.mood);
}

void Bulldog::setMood(Mood m) {
    _mood = m;
    if (m == MOOD_WAVE) {
        _pawFrame    = 0;
        _pawNextTick = millis();
    } else {
        _pawFrame = -1;
    }
    renderFace();
}

void Bulldog::tick() {
    uint32_t now = millis();
    bool dirty = false;

    // Idle blinking (applies in every mood except sleepy/smoke, where
    // the eye shape is already special-cased in renderFace).
    if (_mood != MOOD_SLEEPY && _mood != MOOD_SMOKE) {
        if (!_eyesClosed && now >= _nextBlinkAt) {
            _eyesClosed  = true;
            _blinkEndsAt = now + 140;
            dirty = true;
        } else if (_eyesClosed && now >= _blinkEndsAt) {
            _eyesClosed  = false;
            _nextBlinkAt = now + 2500 + random(3500);
            dirty = true;
        }
    } else if (_eyesClosed) {
        _eyesClosed = false;
        dirty = true;
    }

    // Wave paw animation
    if (_mood == MOOD_WAVE && now >= _pawNextTick) {
        _pawFrame    = (_pawFrame + 1) % 4;
        _pawNextTick = now + 180;
        dirty = true;
    }

    if (dirty) renderFace();
}

// ---------- drawing helpers ----------

void Bulldog::drawPaw() {
    int baseX = 200;
    int baseY;
    switch (_pawFrame % 4) {
        case 0: baseY = 50; break;
        case 1: baseY = 30; break;
        case 2: baseY = 20; break;
        case 3: baseY = 35; break;
        default: baseY = 45;
    }
    _faceSpr.fillRoundRect(baseX, baseY + 10, 34, 22, 8, FUR);
    _faceSpr.fillCircle(baseX + 6,  baseY + 6, 6, FUR);
    _faceSpr.fillCircle(baseX + 17, baseY + 2, 6, FUR);
    _faceSpr.fillCircle(baseX + 28, baseY + 6, 6, FUR);
    _faceSpr.fillCircle(baseX + 6,  baseY + 6, 2, PINK);
    _faceSpr.fillCircle(baseX + 17, baseY + 2, 2, PINK);
    _faceSpr.fillCircle(baseX + 28, baseY + 6, 2, PINK);
}

void Bulldog::drawRaisedPaws() {
    int py = 4;
    int xs[2] = { 40, 190 };
    for (int i = 0; i < 2; i++) {
        int px = xs[i];
        _faceSpr.fillRoundRect(px, py + 12, 34, 22, 8, FUR);
        _faceSpr.fillCircle(px + 6,  py + 8, 6, FUR);
        _faceSpr.fillCircle(px + 17, py + 4, 6, FUR);
        _faceSpr.fillCircle(px + 28, py + 8, 6, FUR);
    }
}

void Bulldog::drawCoffeeCup() {
    int cx = 200, cy = 18;
    _faceSpr.fillRoundRect(cx, cy, 34, 30, 4, TFT_WHITE);
    _faceSpr.drawRoundRect(cx, cy, 34, 30, 4, TFT_BLACK);
    _faceSpr.fillRoundRect(cx + 2, cy + 2, 30, 8, 2, BROWN);
    _faceSpr.drawCircle(cx + 38, cy + 15, 7, TFT_WHITE);
    _faceSpr.drawCircle(cx + 38, cy + 15, 6, TFT_BLACK);
    for (int i = 0; i < 3; i++) {
        int sx = cx + 6 + i * 11;
        _faceSpr.drawLine(sx,     cy - 4,  sx + 3, cy - 10, TFT_WHITE);
        _faceSpr.drawLine(sx + 3, cy - 10, sx,     cy - 16, TFT_WHITE);
    }
}

void Bulldog::renderFace() {
    _faceSpr.fillSprite(BG);

    // ---- Ears (behind head)
    _faceSpr.fillTriangle(30,  50, 55,  0, 92,  42, FUR);
    _faceSpr.fillTriangle(48,  42, 60, 12, 78,  40, PINK);
    _faceSpr.fillTriangle(168, 42, 205, 0, 230, 50, FUR);
    _faceSpr.fillTriangle(182, 40, 200, 12, 212, 42, PINK);

    // ---- Head
    _faceSpr.fillRoundRect(25, 35, 210, 130, 55, FUR);

    // ---- Wrinkle / nose bridge
    _faceSpr.fillTriangle(122, 68, 138, 68, 130, 92, FUR_DARK);
    _faceSpr.drawFastHLine(118, 78, 24, FUR_DARK);
    _faceSpr.drawFastHLine(120, 85, 20, FUR_DARK);

    // ---- Snout
    _faceSpr.fillRoundRect(70, 98, 120, 58, 28, SNOUT);

    // ---- Eyes
    bool sleepy = (_mood == MOOD_SLEEPY);
    bool wink   = (_mood == MOOD_SMOKE);

    if (_eyesClosed) {
        _faceSpr.fillRect(74,  78, 34, 3, TFT_BLACK);
        _faceSpr.fillRect(154, 78, 34, 3, TFT_BLACK);
    } else if (sleepy) {
        _faceSpr.fillCircle(90,  78, 16, TFT_BLACK);
        _faceSpr.fillCircle(170, 78, 16, TFT_BLACK);
        _faceSpr.fillRect(70,  62, 40, 18, FUR);
        _faceSpr.fillRect(150, 62, 40, 18, FUR);
        _faceSpr.drawFastHLine(74,  80, 34, TFT_BLACK);
        _faceSpr.drawFastHLine(154, 80, 34, TFT_BLACK);
    } else {
        _faceSpr.fillCircle(90, 78, 16, TFT_BLACK);
        _faceSpr.fillCircle(94, 73, 4,  TFT_WHITE);
        if (wink) {
            _faceSpr.fillRect(154, 78, 34, 3, TFT_BLACK);
            _faceSpr.drawLine(186, 78, 190, 74, TFT_BLACK);
        } else {
            _faceSpr.fillCircle(170, 78, 16, TFT_BLACK);
            _faceSpr.fillCircle(174, 73, 4,  TFT_WHITE);
        }
    }

    // ---- Nose
    _faceSpr.fillEllipse(130, 108, 22, 12, TFT_BLACK);
    _faceSpr.fillEllipse(124, 104, 3,  2,  TFT_WHITE);

    // ---- Mouth
    if (_mood == MOOD_SLEEPY) {
        _faceSpr.fillEllipse(130, 140, 20, 14, TFT_BLACK);
        _faceSpr.fillEllipse(130, 143, 12,  8, PINK);
    } else {
        _faceSpr.drawWideLine(130, 118, 130, 140, 3, TFT_BLACK);
        _faceSpr.drawWideLine(130, 140, 108, 148, 3, TFT_BLACK);
        _faceSpr.drawWideLine(130, 140, 152, 148, 3, TFT_BLACK);

        if (_mood == MOOD_HAPPY || _mood == MOOD_COFFEE || _mood == MOOD_WAVE) {
            _faceSpr.fillRoundRect(120, 145, 20, 12, 4, PINK);
            _faceSpr.drawFastVLine(130, 148, 8, PINK_DK);
        }
    }

    // ---- Accessories / poses
    if (_mood == MOOD_COFFEE)  drawCoffeeCup();
    if (_mood == MOOD_STRETCH) drawRaisedPaws();
    if (_mood == MOOD_WAVE && _pawFrame >= 0) drawPaw();

    _faceSpr.pushSprite(FACE_X, FACE_Y);
}
