#include "quickdraw.h"

// --- Duel Phase State Machine ---
enum DuelPhase {
    DP_INTRO,
    DP_WAITING,
    DP_DRAW,
    DP_WIN,
    DP_LOSE,
    DP_TOO_EARLY,
    DP_ALL_CLEAR
};

static DuelPhase duelPhase;
static int currentOpponent = 0;
static unsigned long waitStart, waitDelay;
static unsigned long drawTime;
static unsigned long reactionMs;
static int animFrame;
static unsigned long lastFrameTime;
static bool animDone;
static unsigned long phaseEntryTime;  // debounce: ignore input briefly after phase change

// Tone sequencing
struct ToneStep { int freq; int dur; };
static const ToneStep winTones[] = {{800,80},{1200,80},{1600,120}};
static const ToneStep loseTones[] = {{400,150},{200,300}};
static const ToneStep champTones[] = {{1000,100},{1200,100},{1400,100},{1600,150}};
static int toneIdx;
static unsigned long toneStart;
static const ToneStep* toneSeq;
static int toneCount;

static void startToneSeq(const ToneStep* seq, int count) {
    toneSeq = seq;
    toneCount = count;
    toneIdx = 0;
    toneStart = millis();
    M5.Speaker.tone(seq[0].freq, seq[0].dur);
}

static void updateToneSeq() {
    if (!toneSeq || toneIdx >= toneCount) return;
    if (millis() - toneStart >= (unsigned long)toneSeq[toneIdx].dur + 30) {
        toneIdx++;
        if (toneIdx < toneCount) {
            toneStart = millis();
            M5.Speaker.tone(toneSeq[toneIdx].freq, toneSeq[toneIdx].dur);
        } else {
            toneSeq = nullptr;
        }
    }
}

// --- Colors ---
#define COL_BROWN    0x8A22
#define COL_DKBROWN  0x4A00
#define COL_SKIN     0xFDD0
#define COL_DKSKIN   0xDB80
#define COL_MAROON   0x8000
#define COL_PALE     0xDEDB

// --- Opponent Drawing Functions ---
// All draw at horizontal center cx, with vertical offset yOff
// If dead is true, draw X eyes instead of normal eyes

static void drawXEyes(int cx, int yOff, int eyeY) {
    // Left X eye
    M5.Display.drawLine(cx - 9, eyeY - 3 + yOff, cx - 3, eyeY + 3 + yOff, RED);
    M5.Display.drawLine(cx - 3, eyeY - 3 + yOff, cx - 9, eyeY + 3 + yOff, RED);
    // Right X eye
    M5.Display.drawLine(cx + 3, eyeY - 3 + yOff, cx + 9, eyeY + 3 + yOff, RED);
    M5.Display.drawLine(cx + 9, eyeY - 3 + yOff, cx + 3, eyeY + 3 + yOff, RED);
}

static void drawDustyDan(int cx, int yOff, bool dead = false) {
    yOff += 14;
    // Tall brown cowboy hat
    M5.Display.fillRect(cx - 28, 8 + yOff, 56, 6, COL_BROWN);   // brim
    M5.Display.fillRect(cx - 14, -4 + yOff, 28, 14, COL_BROWN); // crown
    // Head
    M5.Display.fillCircle(cx, 28 + yOff, 16, COL_SKIN);
    // Eyes
    if (dead) { drawXEyes(cx, yOff, 25); }
    else { M5.Display.fillCircle(cx - 6, 25 + yOff, 2, BLACK); M5.Display.fillCircle(cx + 6, 25 + yOff, 2, BLACK); }
    // Mouth
    M5.Display.drawLine(cx - 4, 35 + yOff, cx + 4, 35 + yOff, COL_DKBROWN);
    // Body - brown vest
    M5.Display.fillRect(cx - 18, 45 + yOff, 36, 30, COL_BROWN);
    // Arms
    M5.Display.fillRect(cx - 26, 48 + yOff, 10, 22, COL_SKIN);
    M5.Display.fillRect(cx + 16, 48 + yOff, 10, 22, COL_SKIN);
    // Legs
    M5.Display.fillRect(cx - 12, 75 + yOff, 10, 18, BLUE);
    M5.Display.fillRect(cx + 2, 75 + yOff, 10, 18, BLUE);
}

static void drawWhiskeyPete(int cx, int yOff, bool dead = false) {
    yOff += 14;
    // Flat black hat
    M5.Display.fillRect(cx - 24, 10 + yOff, 48, 5, BLACK);  // brim
    M5.Display.fillRect(cx - 12, 0 + yOff, 24, 12, BLACK);  // crown
    // Head
    M5.Display.fillCircle(cx, 28 + yOff, 16, COL_SKIN);
    // Eyes
    if (dead) { drawXEyes(cx, yOff, 25); }
    else { M5.Display.fillCircle(cx - 6, 25 + yOff, 2, BLACK); M5.Display.fillCircle(cx + 6, 25 + yOff, 2, BLACK); }
    // Thick mustache
    M5.Display.fillRect(cx - 10, 32 + yOff, 20, 4, COL_DKBROWN);
    // Body - red shirt
    M5.Display.fillRect(cx - 18, 45 + yOff, 36, 30, RED);
    // Arms
    M5.Display.fillRect(cx - 26, 48 + yOff, 10, 22, RED);
    M5.Display.fillRect(cx + 16, 48 + yOff, 10, 22, RED);
    // Legs
    M5.Display.fillRect(cx - 12, 75 + yOff, 10, 18, COL_DKBROWN);
    M5.Display.fillRect(cx + 2, 75 + yOff, 10, 18, COL_DKBROWN);
}

static void drawOneEyeJack(int cx, int yOff, bool dead = false) {
    yOff += 14;
    // Medium brown hat
    M5.Display.fillRect(cx - 22, 10 + yOff, 44, 5, COL_DKBROWN); // brim
    M5.Display.fillRect(cx - 12, -2 + yOff, 24, 14, COL_DKBROWN); // crown
    // Head
    M5.Display.fillCircle(cx, 28 + yOff, 16, COL_SKIN);
    // Eyes
    if (dead) { drawXEyes(cx, yOff, 25); }
    else {
        M5.Display.fillCircle(cx + 6, 25 + yOff, 2, BLACK); // Right eye
        M5.Display.fillRect(cx - 10, 22 + yOff, 8, 7, BLACK); // Left eye patch
        M5.Display.drawLine(cx - 6, 22 + yOff, cx + 10, 14 + yOff, BLACK); // Strap
    }
    // Scar on right cheek
    M5.Display.drawLine(cx + 10, 22 + yOff, cx + 14, 34 + yOff, RED);
    // Mouth
    M5.Display.drawLine(cx - 3, 35 + yOff, cx + 3, 36 + yOff, COL_DKBROWN);
    // Body - dark green vest
    uint16_t dkGreen = 0x03E0;
    M5.Display.fillRect(cx - 18, 45 + yOff, 36, 30, dkGreen);
    // Arms
    M5.Display.fillRect(cx - 26, 48 + yOff, 10, 22, COL_SKIN);
    M5.Display.fillRect(cx + 16, 48 + yOff, 10, 22, COL_SKIN);
    // Legs
    M5.Display.fillRect(cx - 12, 75 + yOff, 10, 18, COL_DKBROWN);
    M5.Display.fillRect(cx + 2, 75 + yOff, 10, 18, COL_DKBROWN);
}

static void drawElSerpiente(int cx, int yOff, bool dead = false) {
    yOff += 14;
    // Wide sombrero
    M5.Display.fillTriangle(cx - 34, 16 + yOff, cx + 34, 16 + yOff, cx, -4 + yOff, COL_DKBROWN);
    M5.Display.fillRect(cx - 12, 2 + yOff, 24, 8, YELLOW); // gold band
    M5.Display.fillCircle(cx, 6 + yOff, 12, COL_DKBROWN); // dome
    // Head
    M5.Display.fillCircle(cx, 30 + yOff, 16, COL_DKSKIN);
    // Eyes
    if (dead) { drawXEyes(cx, yOff, 27); }
    else { M5.Display.fillRect(cx - 8, 27 + yOff, 5, 3, BLACK); M5.Display.fillRect(cx + 3, 27 + yOff, 5, 3, BLACK); }
    // Thin mustache
    M5.Display.drawLine(cx - 10, 35 + yOff, cx - 1, 33 + yOff, BLACK);
    M5.Display.drawLine(cx + 1, 33 + yOff, cx + 10, 35 + yOff, BLACK);
    // Body - maroon
    M5.Display.fillRect(cx - 18, 47 + yOff, 36, 30, COL_MAROON);
    // Arms
    M5.Display.fillRect(cx - 26, 50 + yOff, 10, 22, COL_MAROON);
    M5.Display.fillRect(cx + 16, 50 + yOff, 10, 22, COL_MAROON);
    // Legs
    M5.Display.fillRect(cx - 12, 77 + yOff, 10, 16, BLACK);
    M5.Display.fillRect(cx + 2, 77 + yOff, 10, 16, BLACK);
}

static void drawBlackBart(int cx, int yOff, bool dead = false) {
    yOff += 14;
    // Tall menacing black hat
    M5.Display.fillRect(cx - 26, 10 + yOff, 52, 6, BLACK);  // brim
    M5.Display.fillRect(cx - 14, -2 + yOff, 28, 14, BLACK); // crown
    // Head
    M5.Display.fillCircle(cx, 28 + yOff, 16, COL_SKIN);
    // Eyes
    if (dead) { drawXEyes(cx, yOff, 24); }
    else {
        M5.Display.fillRect(cx - 8, 24 + yOff, 6, 3, BLACK);
        M5.Display.fillRect(cx + 2, 24 + yOff, 6, 3, BLACK);
        M5.Display.drawLine(cx - 9, 21 + yOff, cx - 3, 22 + yOff, BLACK);
        M5.Display.drawLine(cx + 3, 22 + yOff, cx + 9, 21 + yOff, BLACK);
    }
    // Red bandana over lower face
    M5.Display.fillTriangle(cx - 14, 30 + yOff, cx + 14, 30 + yOff, cx, 42 + yOff, RED);
    // Body - black duster
    M5.Display.fillRect(cx - 20, 45 + yOff, 40, 32, 0x1082); // very dark gray
    // Arms
    M5.Display.fillRect(cx - 28, 48 + yOff, 10, 24, 0x1082);
    M5.Display.fillRect(cx + 18, 48 + yOff, 10, 24, 0x1082);
    // Legs
    M5.Display.fillRect(cx - 12, 77 + yOff, 10, 16, BLACK);
    M5.Display.fillRect(cx + 2, 77 + yOff, 10, 16, BLACK);
}

static void drawUndertaker(int cx, int yOff, bool dead = false) {
    yOff += 14;
    // Tall top hat
    M5.Display.fillRect(cx - 16, 8 + yOff, 32, 5, BLACK);  // brim
    M5.Display.fillRect(cx - 10, -4 + yOff, 20, 18, BLACK); // tall crown
    // Head - pale
    M5.Display.fillCircle(cx, 28 + yOff, 16, COL_PALE);
    // Eyes
    if (dead) { drawXEyes(cx, yOff, 26); }
    else { M5.Display.fillCircle(cx - 6, 26 + yOff, 4, BLACK); M5.Display.fillCircle(cx + 6, 26 + yOff, 4, BLACK); }
    // Thin grim mouth
    M5.Display.drawLine(cx - 6, 36 + yOff, cx + 6, 36 + yOff, DARKGREY);
    // Body - black suit
    M5.Display.fillRect(cx - 18, 45 + yOff, 36, 32, BLACK);
    // White shirt line
    M5.Display.drawLine(cx, 45 + yOff, cx, 76 + yOff, WHITE);
    // Arms
    M5.Display.fillRect(cx - 26, 48 + yOff, 10, 24, BLACK);
    M5.Display.fillRect(cx + 16, 48 + yOff, 10, 24, BLACK);
    // Legs
    M5.Display.fillRect(cx - 12, 77 + yOff, 10, 16, 0x1082);
    M5.Display.fillRect(cx + 2, 77 + yOff, 10, 16, 0x1082);
}

// --- Opponent Table ---
typedef void (*DrawFn)(int cx, int yOff, bool dead);

struct Opponent {
    const char* name;
    int reactionMs;
    DrawFn draw;
    const char* speedLabel;
    uint16_t speedColor;
};

static const int NUM_OPPONENTS = 6;
static const Opponent opponents[NUM_OPPONENTS] = {
    {"Dusty Dan",       800, drawDustyDan,      "Slow",      0x03A0},
    {"Mad Pete",        600, drawWhiskeyPete,    "Moderate",  0x0410},
    {"One-Eye",         450, drawOneEyeJack,     "Quick",     0xC600},
    {"Serpiente",       340, drawElSerpiente,     "Fast",      ORANGE},
    {"Black Bart",      260, drawBlackBart,      "Deadly",    RED},
    {"Undertaker",      200, drawUndertaker,     "Legendary", 0x8010},
};

// --- Helper: draw tombstone (lose screen) — same style as win gravestone ---
static void drawTombstone() {
    int cx = 67;
    int stoneW = 54;
    int groundY = 180;
    int topY = 105;
    int archY = topY + 12;
    // Arch
    M5.Display.fillCircle(cx, archY, stoneW/2, DARKGREY);
    // Body
    M5.Display.fillRect(cx - stoneW/2, archY, stoneW, groundY - archY, DARKGREY);
    // Clip below stone
    M5.Display.fillRect(0, groundY, 135, 25, WHITE);
    // Cross
    M5.Display.fillRect(cx - 2, topY, 4, 14, LIGHTGREY);
    M5.Display.fillRect(cx - 6, topY + 4, 12, 3, LIGHTGREY);
    // R.I.P.
    M5.Display.setTextColor(WHITE, DARKGREY);
    M5.Display.setTextSize(1);
    M5.Display.setCursor(cx - 12, topY + 28);
    M5.Display.print("R.I.P.");
}

// --- Helper: draw ghost ---
static void drawGhost(int cx, int yOff) {
    // Body
    M5.Display.fillCircle(cx, 30 + yOff, 20, LIGHTGREY);     // head
    M5.Display.fillRect(cx - 20, 30 + yOff, 40, 45, LIGHTGREY); // body
    // Wavy bottom - 3 scallops cut out with white circles
    M5.Display.fillCircle(cx - 13, 75 + yOff, 7, WHITE);
    M5.Display.fillCircle(cx + 1, 75 + yOff, 7, WHITE);
    M5.Display.fillCircle(cx + 15, 75 + yOff, 7, WHITE);
    // X eyes in dark grey
    M5.Display.drawLine(cx - 9, 25 + yOff, cx - 3, 31 + yOff, DARKGREY);
    M5.Display.drawLine(cx - 3, 25 + yOff, cx - 9, 31 + yOff, DARKGREY);
    M5.Display.drawLine(cx + 3, 25 + yOff, cx + 9, 31 + yOff, DARKGREY);
    M5.Display.drawLine(cx + 9, 25 + yOff, cx + 3, 31 + yOff, DARKGREY);
    // Open mouth - small oval
    M5.Display.fillCircle(cx, 42 + yOff, 4, DARKGREY);
}

// --- Helper: draw gravestone for win animation ---
static void drawWinGravestone(int cx, int topY) {
    int stoneW = 54;
    int groundY = 93;
    int archY = topY + 12;  // center of arch
    // Draw arch (top rounded part)
    M5.Display.fillCircle(cx, archY, stoneW/2, DARKGREY);
    // Draw rectangular stone body below arch down to ground
    int bodyTop = archY;
    int bodyH = groundY - bodyTop;
    if (bodyH > 0) {
        M5.Display.fillRect(cx - stoneW/2, bodyTop, stoneW, bodyH, DARKGREY);
    }
    // Clip below stone
    M5.Display.fillRect(0, groundY, 135, 45, WHITE);
    // Cross
    if (topY < 60) {
        M5.Display.fillRect(cx - 2, topY, 4, 14, LIGHTGREY);
        M5.Display.fillRect(cx - 6, topY + 4, 12, 3, LIGHTGREY);
    }
    // R.I.P.
    if (topY + 28 < groundY) {
        M5.Display.setTextColor(WHITE, DARKGREY);
        M5.Display.setTextSize(1);
        M5.Display.setCursor(cx - 12, topY + 28);
        M5.Display.print("R.I.P.");
    }
}

// --- Helper: draw sheriff badge ---
static void drawBadge(int cx, int cy) {
    // Star shape from overlapping triangles
    int r = 22;
    M5.Display.fillTriangle(cx, cy - r, cx - r + 4, cy + r/2 + 2, cx + r - 4, cy + r/2 + 2, YELLOW);
    M5.Display.fillTriangle(cx, cy + r, cx - r + 4, cy - r/2 - 2, cx + r - 4, cy - r/2 - 2, YELLOW);
    M5.Display.fillCircle(cx, cy, 8, ORANGE);
}

// --- Phase transition helpers ---
static void showIntro() {
    duelPhase = DP_INTRO;
    const Opponent& opp = opponents[currentOpponent];

    M5.Display.fillScreen(WHITE);
    // Draw opponent
    opp.draw(67, 0, false);
    // Name
    int nameLen = strlen(opp.name);
    M5.Display.setTextSize(2);
    int nameX = (135 - nameLen * 12) / 2;
    if (nameX < 2) nameX = 2;
    M5.Display.setCursor(nameX, 114);
    M5.Display.setTextColor(BLACK, WHITE);
    M5.Display.print(opp.name);
    // Duel number
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(DARKGREY, WHITE);
    // "Duel N/6" = 8 chars * 6px = 48px
    M5.Display.setCursor((135 - 48) / 2, 140);
    M5.Display.printf("Duel %d/6", currentOpponent + 1);
    // Speed
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(opp.speedColor, WHITE);
    int speedLen = 7 + strlen(opp.speedLabel); // "Speed: " + label
    M5.Display.setCursor((135 - speedLen * 6) / 2, 157);
    M5.Display.printf("Speed: %s", opp.speedLabel);
    // Hints
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(DARKGREY, WHITE);
    M5.Display.setCursor(4, 210);
    M5.Display.print("A:Duel B:");
    drawLeftArrow(DARKGREY);
}

static void showWaiting() {
    duelPhase = DP_WAITING;
    waitDelay = random(2000, 4001);
    waitStart = millis();

    M5.Display.fillScreen(WHITE);
    // Redraw opponent at top
    opponents[currentOpponent].draw(67, 0, false);
    // Steady text
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(RED, WHITE);
    // "STEADY..." = 9 chars * 12px = 108px
    M5.Display.setCursor((135 - 108) / 2, 135);
    M5.Display.print("STEADY...");

    M5.Speaker.tone(300, 150);
}

static void showDraw() {
    duelPhase = DP_DRAW;
    drawTime = millis();

    M5.Display.fillScreen(WHITE);
    // Red border flash
    M5.Display.drawRect(0, 0, 135, 240, RED);
    M5.Display.drawRect(1, 1, 133, 238, RED);
    M5.Display.drawRect(2, 2, 131, 236, RED);
    // DRAW! text
    M5.Display.setTextSize(3);
    M5.Display.setTextColor(RED, WHITE);
    // "DRAW!" = 5 chars * 18px = 90px
    M5.Display.setCursor((135 - 90) / 2, 110);
    M5.Display.print("DRAW!");

    M5.Speaker.tone(2000, 50);
}

static void showWin() {
    duelPhase = DP_WIN;
    animFrame = 0;
    animDone = false;
    lastFrameTime = millis();
    toneSeq = nullptr;

    M5.Display.fillScreen(WHITE);

    // Draw opponent with X eyes
    opponents[currentOpponent].draw(67, 0, true);

    // Praise (big)
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(0x03A0, WHITE);
    const char* praise;
    if (reactionMs < 200) praise = "LIGHTNING!";
    else if (reactionMs < 350) praise = "Incredible!";
    else praise = "Got 'em!";
    M5.Display.setCursor((135 - (int)strlen(praise) * 12) / 2, 150);
    M5.Display.print(praise);

    // Reaction time (small)
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(DARKGREY, WHITE);
    char timeBuf[16];
    snprintf(timeBuf, sizeof(timeBuf), "%lu ms", reactionMs);
    M5.Display.setCursor((135 - (int)strlen(timeBuf) * 6) / 2, 178);
    M5.Display.print(timeBuf);

    startToneSeq(winTones, 3);
}

static void showLose() {
    duelPhase = DP_LOSE;
    phaseEntryTime = millis();

    M5.Display.fillScreen(WHITE);
    // BANG!
    M5.Display.setTextSize(3);
    M5.Display.setTextColor(RED, WHITE);
    // "BANG!" = 5 chars * 18px = 90px at textSize 3
    M5.Display.setCursor((135 - 90) / 2, 30);
    M5.Display.print("BANG!");

    M5.Display.setTextSize(1);
    M5.Display.setTextColor(RED, WHITE);
    char loseBuf[24];
    snprintf(loseBuf, sizeof(loseBuf), "They shot in %dms", opponents[currentOpponent].reactionMs);
    M5.Display.setCursor((135 - (int)strlen(loseBuf) * 6) / 2, 65);
    M5.Display.print(loseBuf);

    // Draw tombstone
    drawTombstone();

    // Hints
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(DARKGREY, WHITE);
    M5.Display.setCursor(4, 210);
    M5.Display.print("A:Retry B:");
    drawLeftArrow(DARKGREY);

    startToneSeq(loseTones, 2);
}

static void showTooEarly() {
    duelPhase = DP_TOO_EARLY;
    phaseEntryTime = millis();

    M5.Display.fillScreen(RED);
    // Big X
    int xc = 67;
    M5.Display.drawLine(xc - 25, 20, xc + 25, 70, WHITE);
    M5.Display.drawLine(xc + 25, 20, xc - 25, 70, WHITE);
    M5.Display.drawLine(xc - 24, 20, xc + 26, 70, WHITE);
    M5.Display.drawLine(xc + 26, 20, xc - 24, 70, WHITE);

    M5.Display.setTextSize(2);
    M5.Display.setTextColor(WHITE, RED);
    // "FALSE" = 5*12=60, "START!" = 6*12=72
    M5.Display.setCursor((135 - 60) / 2, 90);
    M5.Display.print("FALSE");
    M5.Display.setCursor((135 - 72) / 2, 115);
    M5.Display.print("START!");

    M5.Display.setTextSize(1);
    // "You shot too early!" = 19*6=114
    M5.Display.setCursor((135 - 114) / 2, 150);
    M5.Display.print("You shot too early!");

    M5.Display.setTextSize(2);
    M5.Display.setCursor(4, 210);
    M5.Display.print("A:Retry B:");
    drawLeftArrow(WHITE);

    M5.Speaker.tone(200, 400);
}

static void showVictory() {
    duelPhase = DP_ALL_CLEAR;

    M5.Display.fillScreen(WHITE);
    // Sheriff badge
    drawBadge(67, 45);

    M5.Display.setTextSize(2);
    M5.Display.setTextColor(BLACK, WHITE);
    // "You're the" = 10*12=120, "Sheriff!" = 8*12=96
    M5.Display.setCursor((135 - 120) / 2, 90);
    M5.Display.print("You're the");
    M5.Display.setCursor((135 - 96) / 2, 115);
    M5.Display.print("Sheriff!");

    M5.Display.setTextSize(1);
    M5.Display.setTextColor(DARKGREY, WHITE);
    // "The town is safe." = 17*6=102
    M5.Display.setCursor((135 - 102) / 2, 145);
    M5.Display.print("The town is safe.");

    M5.Display.setTextSize(2);
    M5.Display.setTextColor(DARKGREY, WHITE);
    M5.Display.setCursor(4, 210);
    M5.Display.print("A:Again B:");
    drawLeftArrow(DARKGREY);

    startToneSeq(champTones, 4);
}

// --- Public API ---
void enterQuickDraw() {
    currentOpponent = 0;
    showIntro();
}

void updateQuickDraw() {
    updateToneSeq();

    if (duelPhase != DP_DRAW && duelPhase != DP_WAITING && M5.BtnB.wasPressed()) {
        switchState(STATE_MENU);
        return;
    }

    switch (duelPhase) {
        case DP_INTRO:
            if (M5.BtnA.wasPressed()) {
                showWaiting();
            }
            break;

        case DP_WAITING:
            if (M5.BtnA.wasPressed()) {
                showTooEarly();
            } else if (millis() - waitStart >= waitDelay) {
                showDraw();
            }
            break;

        case DP_DRAW: {
            unsigned long elapsed = millis() - drawTime;
            if (M5.BtnA.wasPressed()) {
                reactionMs = elapsed;
                showWin();
            } else if (elapsed >= (unsigned long)opponents[currentOpponent].reactionMs) {
                showLose();
            }
            break;
        }

        case DP_WIN:
            if (!animDone) {
                if (millis() - lastFrameTime >= 80) {
                    lastFrameTime = millis();
                    animFrame++;

                    if (animFrame < 8) {
                        // Phase 1: hold X-eyes (already drawn)
                    } else if (animFrame == 8) {
                        // Phase 2: swap to ghost
                        M5.Display.fillRect(0, 0, 135, 140, WHITE);
                        drawGhost(67, 0);
                    } else if (animFrame < 28) {
                        // Phase 3: ghost floats up
                        int ghostY = -(animFrame - 8) * 6;
                        M5.Display.fillRect(0, 0, 135, 100, WHITE);
                        if (ghostY > -80) drawGhost(67, ghostY);
                    } else if (animFrame < 42) {
                        // Phase 4: gravestone rises
                        int progress = animFrame - 28; // 0..13
                        int topY = 90 - progress * 5;  // 90 down to ~25
                        if (topY < 25) topY = 25;
                        M5.Display.fillRect(0, 0, 135, 140, WHITE);
                        drawWinGravestone(67, topY);
                    } else {
                        animDone = true;
                        // Show button hints
                        M5.Display.setTextSize(2);
                        M5.Display.setTextColor(DARKGREY, WHITE);
                        M5.Display.setCursor(4, 210);
                        if (currentOpponent >= NUM_OPPONENTS - 1) {
                            M5.Display.print("A:Cont B:");
                        } else {
                            M5.Display.print("A:Next B:");
                        }
                        drawLeftArrow(DARKGREY);
                    }
                }
            } else if (M5.BtnA.wasPressed()) {
                if (currentOpponent >= NUM_OPPONENTS - 1) {
                    showVictory();
                } else {
                    currentOpponent++;
                    showIntro();
                }
            }
            break;

        case DP_LOSE:
        case DP_TOO_EARLY:
            if (M5.BtnA.wasPressed() && millis() - phaseEntryTime > 500) {
                showIntro();
            }
            break;

        case DP_ALL_CLEAR:
            if (M5.BtnA.wasPressed()) {
                currentOpponent = 0;
                showIntro();
            }
            break;
    }
}
