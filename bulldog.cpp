#include "bulldog.h"

#define SCR_W    320
#define SCR_H    240
#define TEXT_Y   180
#define TEXT_H   (SCR_H - TEXT_Y)

// Arduino_GFX exposes RGB565() as a macro for packing 8-bit RGB.
#define COL(r,g,b)  RGB565((r), (g), (b))
#define BG        COL(15,  25,  50)
#define FUR       COL(210, 180, 140)
#define FUR_DARK  COL(160, 130,  95)
#define SNOUT     COL(240, 220, 185)
#define PINK      COL(255, 130, 170)
#define PINK_DK   COL(200,  80, 130)
#define BROWN     COL(90,   55,  25)
#define WHITE     0xFFFF
#define BLACK     0x0000
#define TEXT_L2   COL(180, 200, 230)

Bulldog::Bulldog(Arduino_GFX* display)
    : _display(display),
      _canvas(new Arduino_Canvas(SCR_W, SCR_H, display)) {}

void Bulldog::begin() {
    _canvas->begin();
    _canvas->fillScreen(BG);
    _canvas->flush();
    _nextBlinkAt = millis() + random(2500, 5000);
    renderFace();
}

void Bulldog::drawCenteredText(const char* text, int y, uint8_t size, uint16_t color) {
    if (!text) return;
    _canvas->setTextSize(size);
    _canvas->setTextColor(color);
    int w = (int)strlen(text) * 6 * size;
    int x = (SCR_W - w) / 2;
    if (x < 0) x = 0;
    _canvas->setCursor(x, y);
    _canvas->print(text);
}

void Bulldog::drawText(const char* line1, const char* line2) {
    _canvas->fillRect(0, TEXT_Y, SCR_W, TEXT_H, BG);
    drawCenteredText(line1, TEXT_Y + 6,  3, WHITE);
    drawCenteredText(line2, TEXT_Y + 38, 2, TEXT_L2);
}

void Bulldog::showPrompt(const Prompt& p) {
    drawText(p.line1, p.line2);
    setMood(p.mood);
}

void Bulldog::showCustom(const String& l1, const String& l2,
                         Mood mood, uint32_t holdMs) {
    drawText(l1.c_str(), l2.c_str());
    setMood(mood);
    _holdUntil = millis() + holdMs;
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

bool Bulldog::isHolding() const {
    return millis() < _holdUntil;
}

void Bulldog::tick() {
    uint32_t now = millis();
    bool dirty = false;

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

    if (_mood == MOOD_WAVE && now >= _pawNextTick) {
        _pawFrame    = (_pawFrame + 1) % 4;
        _pawNextTick = now + 180;
        dirty = true;
    }

    if (dirty) renderFace();
}

// ---------- drawing helpers ----------

// Wide line via a few parallel lines. Not anti-aliased but good enough
// for the cartoon mouth.
static void thickLine(Arduino_Canvas* c, int x0, int y0, int x1, int y1,
                      int width, uint16_t color) {
    for (int i = -(width / 2); i <= (width / 2); i++) {
        c->drawLine(x0 + i, y0, x1 + i, y1, color);
        c->drawLine(x0, y0 + i, x1, y1 + i, color);
    }
}

void Bulldog::drawPaw() {
    // Face content is offset within the canvas so the bulldog sits
    // centred above the text strip. The paw waves at the right side
    // of the head. Coordinates are in canvas space.
    int ox = 30;   // horizontal offset for the face group
    int oy = 0;
    int baseX = ox + 200;
    int baseY;
    switch (_pawFrame % 4) {
        case 0: baseY = oy + 50; break;
        case 1: baseY = oy + 30; break;
        case 2: baseY = oy + 20; break;
        case 3: baseY = oy + 35; break;
        default: baseY = oy + 45;
    }
    _canvas->fillRoundRect(baseX, baseY + 10, 34, 22, 8, FUR);
    _canvas->fillCircle(baseX + 6,  baseY + 6, 6, FUR);
    _canvas->fillCircle(baseX + 17, baseY + 2, 6, FUR);
    _canvas->fillCircle(baseX + 28, baseY + 6, 6, FUR);
    _canvas->fillCircle(baseX + 6,  baseY + 6, 2, PINK);
    _canvas->fillCircle(baseX + 17, baseY + 2, 2, PINK);
    _canvas->fillCircle(baseX + 28, baseY + 6, 2, PINK);
}

void Bulldog::drawRaisedPaws() {
    int ox = 30, oy = 0;
    int py = oy + 4;
    int xs[2] = { ox + 40, ox + 190 };
    for (int i = 0; i < 2; i++) {
        int px = xs[i];
        _canvas->fillRoundRect(px, py + 12, 34, 22, 8, FUR);
        _canvas->fillCircle(px + 6,  py + 8, 6, FUR);
        _canvas->fillCircle(px + 17, py + 4, 6, FUR);
        _canvas->fillCircle(px + 28, py + 8, 6, FUR);
    }
}

void Bulldog::drawCoffeeCup() {
    int ox = 30, oy = 0;
    int cx = ox + 200, cy = oy + 18;
    _canvas->fillRoundRect(cx, cy, 34, 30, 4, WHITE);
    _canvas->drawRoundRect(cx, cy, 34, 30, 4, BLACK);
    _canvas->fillRoundRect(cx + 2, cy + 2, 30, 8, 2, BROWN);
    _canvas->drawCircle(cx + 38, cy + 15, 7, WHITE);
    _canvas->drawCircle(cx + 38, cy + 15, 6, BLACK);
    for (int i = 0; i < 3; i++) {
        int sx = cx + 6 + i * 11;
        _canvas->drawLine(sx,     cy - 4,  sx + 3, cy - 10, WHITE);
        _canvas->drawLine(sx + 3, cy - 10, sx,     cy - 16, WHITE);
    }
}

void Bulldog::renderFace() {
    // Clear the whole canvas, then re-draw text (unchanged strings) and
    // face into it, then push the framebuffer to the display in one go.
    _canvas->fillRect(0, 0, SCR_W, TEXT_Y, BG);

    int ox = 30, oy = 0;

    // ---- Ears (behind head)
    _canvas->fillTriangle(ox+30,  oy+50, ox+55,  oy+0,  ox+92,  oy+42, FUR);
    _canvas->fillTriangle(ox+48,  oy+42, ox+60,  oy+12, ox+78,  oy+40, PINK);
    _canvas->fillTriangle(ox+168, oy+42, ox+205, oy+0,  ox+230, oy+50, FUR);
    _canvas->fillTriangle(ox+182, oy+40, ox+200, oy+12, ox+212, oy+42, PINK);

    // ---- Head
    _canvas->fillRoundRect(ox+25, oy+35, 210, 130, 55, FUR);

    // ---- Wrinkle / nose bridge
    _canvas->fillTriangle(ox+122, oy+68, ox+138, oy+68, ox+130, oy+92, FUR_DARK);
    _canvas->drawFastHLine(ox+118, oy+78, 24, FUR_DARK);
    _canvas->drawFastHLine(ox+120, oy+85, 20, FUR_DARK);

    // ---- Snout
    _canvas->fillRoundRect(ox+70, oy+98, 120, 58, 28, SNOUT);

    // ---- Eyes
    bool sleepy = (_mood == MOOD_SLEEPY);
    bool wink   = (_mood == MOOD_SMOKE);

    if (_eyesClosed) {
        _canvas->fillRect(ox+74,  oy+78, 34, 3, BLACK);
        _canvas->fillRect(ox+154, oy+78, 34, 3, BLACK);
    } else if (sleepy) {
        _canvas->fillCircle(ox+90,  oy+78, 16, BLACK);
        _canvas->fillCircle(ox+170, oy+78, 16, BLACK);
        _canvas->fillRect(ox+70,  oy+62, 40, 18, FUR);
        _canvas->fillRect(ox+150, oy+62, 40, 18, FUR);
        _canvas->drawFastHLine(ox+74,  oy+80, 34, BLACK);
        _canvas->drawFastHLine(ox+154, oy+80, 34, BLACK);
    } else {
        _canvas->fillCircle(ox+90, oy+78, 16, BLACK);
        _canvas->fillCircle(ox+94, oy+73, 4,  WHITE);
        if (wink) {
            _canvas->fillRect(ox+154, oy+78, 34, 3, BLACK);
            _canvas->drawLine(ox+186, oy+78, ox+190, oy+74, BLACK);
        } else {
            _canvas->fillCircle(ox+170, oy+78, 16, BLACK);
            _canvas->fillCircle(ox+174, oy+73, 4,  WHITE);
        }
    }

    // ---- Nose
    _canvas->fillEllipse(ox+130, oy+108, 22, 12, BLACK);
    _canvas->fillEllipse(ox+124, oy+104, 3,  2,  WHITE);

    // ---- Mouth
    if (_mood == MOOD_SLEEPY) {
        _canvas->fillEllipse(ox+130, oy+140, 20, 14, BLACK);
        _canvas->fillEllipse(ox+130, oy+143, 12,  8, PINK);
    } else {
        _canvas->fillRect(ox+129, oy+118, 3, 22, BLACK);
        thickLine(_canvas, ox+130, oy+140, ox+108, oy+148, 3, BLACK);
        thickLine(_canvas, ox+130, oy+140, ox+152, oy+148, 3, BLACK);

        if (_mood == MOOD_HAPPY || _mood == MOOD_COFFEE || _mood == MOOD_WAVE) {
            _canvas->fillRoundRect(ox+120, oy+145, 20, 12, 4, PINK);
            _canvas->drawFastVLine(ox+130, oy+148, 8, PINK_DK);
        }
    }

    // ---- Accessories / poses
    if (_mood == MOOD_COFFEE)  drawCoffeeCup();
    if (_mood == MOOD_STRETCH) drawRaisedPaws();
    if (_mood == MOOD_WAVE && _pawFrame >= 0) drawPaw();

    _canvas->flush();
}
