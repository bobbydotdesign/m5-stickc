#include <M5Unified.h>

// --- State machine ---
enum AppState {
    STATE_MENU,
    STATE_REACTION,
    STATE_MAGIC8BALL,
    STATE_TILTMAZE
};

AppState currentState = STATE_MENU;

// --- Menu ---
const char* menuItems[] = { "Reaction", "8-Ball", "Tilt Maze" };
const int menuCount = 3;
int menuSelection = 0;

// Forward declarations
void enterMenu();
void updateMenu();
void enterReaction();
void updateReaction();
void enterMagic8();
void updateMagic8();
void enterMaze();
void updateMaze();

// --- Arrow drawing helpers ---
void drawDownArrow(uint16_t color) {
    int x = M5.Display.getCursorX();
    int y = M5.Display.getCursorY();
    M5.Display.fillTriangle(x + 4, y + 2, x + 12, y + 2, x + 8, y + 12, color);
    M5.Display.setCursor(x + 16, y);
}

void drawRightArrow(uint16_t color) {
    int x = M5.Display.getCursorX();
    int y = M5.Display.getCursorY();
    M5.Display.fillTriangle(x + 2, y + 2, x + 2, y + 12, x + 12, y + 7, color);
    M5.Display.setCursor(x + 16, y);
}

void drawLeftArrow(uint16_t color) {
    int x = M5.Display.getCursorX();
    int y = M5.Display.getCursorY();
    M5.Display.fillTriangle(x + 12, y + 2, x + 12, y + 12, x + 2, y + 7, color);
    M5.Display.setCursor(x + 16, y);
}

// Draw "B:<left arrow>" back hint at current cursor
void drawBackHint(uint16_t color) {
    M5.Display.print("B:");
    drawLeftArrow(color);
}

// Draw "A:Again B:<left arrow>" hint at standard bottom position
void drawRetryBackHint(uint16_t color) {
    M5.Display.setCursor(4, 210);
    M5.Display.print("A:Again B:");
    drawLeftArrow(color);
}

void switchState(AppState newState) {
    currentState = newState;
    switch (newState) {
        case STATE_MENU:      enterMenu();     break;
        case STATE_REACTION:  enterReaction();  break;
        case STATE_MAGIC8BALL: enterMagic8();   break;
        case STATE_TILTMAZE:  enterMaze();      break;
    }
}

// --- Menu ---

void drawMenuItems() {
    M5.Display.setTextSize(2);
    for (int i = 0; i < menuCount; i++) {
        int y = 55 + i * 45;
        if (i == menuSelection) {
            M5.Display.fillRect(0, y - 2, 135, 30, BLUE);
            M5.Display.setTextColor(WHITE, BLUE);
        } else {
            M5.Display.fillRect(0, y - 2, 135, 30, BLACK);
            M5.Display.setTextColor(LIGHTGREY, BLACK);
        }
        M5.Display.setCursor(8, y + 4);
        M5.Display.print(menuItems[i]);
    }
}

void enterMenu() {
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextColor(GREEN, BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(4, 10);
    M5.Display.print("Pick Game");
    M5.Display.drawLine(0, 35, 135, 35, GREEN);
    drawMenuItems();
    // Navigation hint at bottom
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(DARKGREY, BLACK);
    M5.Display.setCursor(4, 210);
    M5.Display.print("A:");
    drawDownArrow(DARKGREY);
    M5.Display.print(" B:");
    drawRightArrow(DARKGREY);
}

void updateMenu() {
    if (M5.BtnA.wasPressed()) {
        menuSelection = (menuSelection + 1) % menuCount;
        drawMenuItems();
    }
    if (M5.BtnB.wasPressed()) {
        switch (menuSelection) {
            case 0: switchState(STATE_REACTION);  break;
            case 1: switchState(STATE_MAGIC8BALL); break;
            case 2: switchState(STATE_TILTMAZE);  break;
        }
    }
}

// --- Stub helper for unfinished apps ---

void showStub(const char* name) {
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextColor(GREEN, BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(8, 10);
    M5.Display.print(name);
    M5.Display.drawLine(0, 35, 135, 35, GREEN);
    M5.Display.setTextColor(WHITE, BLACK);
    M5.Display.setCursor(8, 80);
    M5.Display.print("Coming");
    M5.Display.setCursor(8, 105);
    M5.Display.print("soon!");
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(DARKGREY, BLACK);
    M5.Display.setCursor(4, 210);
    drawBackHint(DARKGREY);
}

// --- Reaction Timer ---

enum ReactionPhase { RT_WAITING, RT_GO, RT_RESULT, RT_TOO_EARLY };
ReactionPhase rtPhase;
unsigned long rtGoTime;
unsigned long rtDelay;
unsigned long rtWaitStart;
unsigned long rtReactionMs;
unsigned long rtBestMs = 0;

void startReactionRound() {
    rtPhase = RT_WAITING;
    rtDelay = random(2000, 5001);
    rtWaitStart = millis();
    M5.Display.fillScreen(RED);
    M5.Display.setTextColor(WHITE, RED);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(20, 95);
    M5.Display.print("Wait...");
    M5.Display.setTextColor(WHITE, RED);
    M5.Display.setCursor(4, 210);
    drawBackHint(WHITE);
}

void enterReaction() {
    startReactionRound();
}

void updateReaction() {
    if (M5.BtnB.wasPressed()) {
        switchState(STATE_MENU);
        return;
    }

    switch (rtPhase) {
        case RT_WAITING:
            if (M5.BtnA.wasPressed()) {
                rtPhase = RT_TOO_EARLY;
                M5.Display.fillScreen(RED);
                M5.Display.setTextColor(WHITE, RED);
                M5.Display.setTextSize(2);
                M5.Display.setCursor(8, 60);
                M5.Display.print("Too early!");
                M5.Speaker.tone(200, 300);
                M5.Display.setTextSize(2);
                M5.Display.setTextColor(WHITE, RED);
                drawRetryBackHint(WHITE);
            } else if (millis() - rtWaitStart >= rtDelay) {
                rtPhase = RT_GO;
                rtGoTime = millis();
                M5.Display.fillScreen(GREEN);
                M5.Display.setTextColor(BLACK, GREEN);
                M5.Display.setTextSize(2);
                M5.Display.setCursor(50, 95);
                M5.Display.print("GO!");
                M5.Speaker.tone(1500, 80);
            }
            break;

        case RT_GO:
            if (M5.BtnA.wasPressed()) {
                rtPhase = RT_RESULT;
                rtReactionMs = millis() - rtGoTime;
                bool newRecord = (rtBestMs == 0 || rtReactionMs < rtBestMs);
                if (newRecord) rtBestMs = rtReactionMs;
                M5.Display.fillScreen(BLACK);
                M5.Display.setTextColor(WHITE, BLACK);
                M5.Display.setTextSize(3);
                M5.Display.setCursor(8, 30);
                M5.Display.printf("%lu ms", rtReactionMs);
                // Rating
                M5.Display.setTextSize(2);
                M5.Display.setCursor(8, 75);
                if (rtReactionMs < 200) {
                    M5.Display.setTextColor(YELLOW, BLACK);
                    M5.Display.print("Lightning!");
                } else if (rtReactionMs < 350) {
                    M5.Display.setTextColor(GREEN, BLACK);
                    M5.Display.print("Fast!");
                } else if (rtReactionMs < 500) {
                    M5.Display.setTextColor(CYAN, BLACK);
                    M5.Display.print("Good!");
                } else {
                    M5.Display.setTextColor(ORANGE, BLACK);
                    M5.Display.print("Try again!");
                }
                // Record
                M5.Display.setTextSize(2);
                if (newRecord) {
                    M5.Display.setTextColor(YELLOW, BLACK);
                    M5.Display.setCursor(8, 120);
                    M5.Display.print("Hi-score!");
                } else {
                    M5.Display.setTextColor(DARKGREY, BLACK);
                    M5.Display.setCursor(8, 120);
                    M5.Display.printf("Record:%lums", rtBestMs);
                }
                M5.Display.setTextSize(2);
                M5.Display.setTextColor(WHITE, BLACK);
                drawRetryBackHint(WHITE);
            }
            break;

        case RT_RESULT:
        case RT_TOO_EARLY:
            if (M5.BtnA.wasPressed()) {
                startReactionRound();
            }
            break;
    }
}

// --- Magic 8-Ball ---

const char* magic8Answers[] = {
    // Affirmative
    "It is certain", "Decidedly so", "Without a doubt",
    "Yes definitely", "Rely on it", "As I see it yes",
    "Most likely", "Outlook good", "Yes",
    "Signs say yes",
    // Non-committal
    "Hazy try again", "Ask again later",
    "Better not say", "Cannot predict now",
    "Focus ask again",
    // Negative
    "Don't count on it", "My reply is no",
    "Sources say no", "Outlook not good",
    "Very doubtful"
};
const int magic8Count = 20;

enum Magic8Phase { M8_IDLE, M8_THINKING, M8_ANSWER };
Magic8Phase m8Phase;
unsigned long m8ThinkStart;
unsigned long m8AnswerTime;
int m8DotCount;
unsigned long m8LastDot;

bool detectShake() {
    M5.Imu.update();
    auto imu = M5.Imu.getImuData();
    float mag = sqrtf(imu.accel.x * imu.accel.x +
                      imu.accel.y * imu.accel.y +
                      imu.accel.z * imu.accel.z);
    return mag > 2.0f;
}

void drawMagic8Idle() {
    M5.Display.fillScreen(BLACK);
    // Large blue triangle
    M5.Display.fillTriangle(10, 30, 125, 30, 67, 170, TFT_NAVY);
    // "8" centered on triangle
    M5.Display.setTextColor(WHITE, TFT_NAVY);
    M5.Display.setTextSize(4);
    M5.Display.setCursor(55, 84);
    M5.Display.print("8");
    // Prompt
    M5.Display.setTextColor(WHITE, BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(14, 180);
    M5.Display.print("Shake me!");
    M5.Display.setCursor(4, 210);
    drawBackHint(WHITE);
}

void enterMagic8() {
    m8Phase = M8_IDLE;
    drawMagic8Idle();
}

void updateMagic8() {
    if (M5.BtnB.wasPressed()) {
        switchState(STATE_MENU);
        return;
    }

    switch (m8Phase) {
        case M8_IDLE:
            if (detectShake()) {
                m8Phase = M8_THINKING;
                m8ThinkStart = millis();
                m8DotCount = 0;
                m8LastDot = 0;
                M5.Display.fillScreen(BLACK);
                M5.Display.setTextColor(WHITE, BLACK);
                M5.Display.setTextSize(2);
                M5.Display.setCursor(30, 100);
                M5.Display.print("Hmm...");
                M5.Speaker.tone(800, 50);
            }
            break;

        case M8_THINKING: {
            unsigned long now = millis();
            // After 1.5s, show answer
            if (now - m8ThinkStart >= 1500) {
                int idx = random(0, magic8Count);
                m8Phase = M8_ANSWER;
                m8AnswerTime = now;
                M5.Display.fillScreen(BLACK);
                // Large blue triangle
                M5.Display.fillTriangle(10, 30, 125, 30, 67, 170, TFT_NAVY);
                // Answer text centered on triangle (transparent bg so text isn't clipped)
                M5.Display.setTextColor(WHITE);
                const char* answer = magic8Answers[idx];
                int len = strlen(answer);
                M5.Display.setTextSize(2);
                // Word-wrap at ~8 chars per line for size 2
                int charsPerLine = 8;
                int lines = (len + charsPerLine - 1) / charsPerLine;
                int startY = 75 - (lines * 10);
                int pos = 0;
                for (int line = 0; line < lines && pos < len; line++) {
                    int end = pos + charsPerLine;
                    if (end >= len) {
                        end = len;
                    } else {
                        int lastSpace = -1;
                        for (int j = pos; j < end; j++) {
                            if (answer[j] == ' ') lastSpace = j;
                        }
                        if (lastSpace > pos) end = lastSpace + 1;
                    }
                    int lineLen = end - pos;
                    if (answer[end - 1] == ' ') lineLen--;
                    int px = 67 - lineLen * 6;
                    M5.Display.setCursor(px, startY + line * 20);
                    for (int j = pos; j < end && answer[j] != '\0'; j++) {
                        if (j == end - 1 && answer[j] == ' ') continue;
                        M5.Display.print(answer[j]);
                    }
                    pos = end;
                }
                // Instructions
                M5.Display.setTextColor(WHITE, BLACK);
                M5.Display.setTextSize(2);
                M5.Display.setCursor(14, 180);
                M5.Display.print("Shake me!");
                M5.Display.setCursor(4, 210);
                drawBackHint(WHITE);
                // Mystical reveal sound
                M5.Speaker.tone(523, 100);
                delay(110);
                M5.Speaker.tone(659, 100);
                delay(110);
                M5.Speaker.tone(784, 200);
            }
            break;
        }

        case M8_ANSWER:
            // Debounce: ignore shakes for 500ms after answer
            if (millis() - m8AnswerTime > 500 && detectShake()) {
                m8Phase = M8_THINKING;
                m8ThinkStart = millis();
                m8DotCount = 0;
                m8LastDot = 0;
                M5.Display.fillScreen(BLACK);
                M5.Display.setTextColor(WHITE, BLACK);
                M5.Display.setTextSize(2);
                M5.Display.setCursor(30, 100);
                M5.Display.print("Hmm...");
                M5.Speaker.tone(800, 50);
            }
            break;
    }
}

// --- Tilt Maze ---

struct Wall {
    int16_t x1, y1, x2, y2;
};

struct Level {
    const Wall* walls;
    uint8_t wallCount;
    float startX, startY;
    float goalX, goalY;
    uint8_t goalSize;
};

// Border walls shared by all levels
const Wall borderWalls[] = {
    {0, 20, 134, 20},     // top
    {0, 20, 0, 239},      // left
    {134, 20, 134, 239},  // right
    {0, 239, 134, 239},   // bottom
};
const int borderCount = 4;

// Level 1: From Figma — two-wall S-curve (gap = 4x ball diameter = 32px)
const Wall lv1Walls[] = {
    {0, 93, 103, 93},         // wall from left, gap right 32px
    {32, 166, 134, 166},      // wall from right, gap left 32px
};

// Level 2: From Figma — three-wall zigzag, equal spacing
const Wall lv2Walls[] = {
    {0, 75, 103, 75},          // wall from left, gap right 32px
    {32, 130, 134, 130},       // wall from right, gap left 32px
    {0, 185, 103, 185},        // wall from left, gap right 32px
};

// Level 3: From Figma — gaps = 6x ball (48px), top-left bar connects to top border
const Wall lv3Walls[] = {
    {45, 20, 45, 43},         // short vertical from TOP BORDER down (connects to y=20)
    {0, 91, 90, 91},          // horizontal wall, left border to right vertical
    {45, 139, 45, 239},       // tall vertical center (48px gap above to H wall)
    {90, 60, 90, 191},        // right vertical (48px gap below to bottom border)
};

// Level 4: "Moving Walls" — no static internal walls, 2 moving walls
const Wall lv4Walls[] = {
    // empty — moving walls handled separately
};
const int lv4WallCount = 0;

// Level 5: "Moving Columns" — no static internal walls, 3 vertical moving walls
const Wall lv5Walls[] = {
    // empty — moving walls handled separately
};
const int lv5WallCount = 0;

// Level 6: "Rotating Arcs" — no static walls, rotating circles handled separately
const Wall lv6Walls[] = {};
const int lv6WallCount = 0;

// Level 7: "Spiral" — walls spiraling inward, 15px corner gaps
const Wall lv7Walls[] = {
    {0, 60, 100, 60},         // H1 from left, gap right 34px
    {100, 75, 100, 105},      // V1: 15px below H1 end, 15px above H2
    {34, 120, 134, 120},      // H2 from right, gap left 34px
    {34, 135, 34, 165},       // V2: 15px below H2 end, 15px above H3
    {0, 180, 100, 180},       // H3 from left, gap right 34px
    {100, 195, 100, 205},     // V3: 15px below H3 end, 15px above H4
    {50, 220, 134, 220},      // H4 from right, gap left 50px
};

// Level 8: "Corridors" — right-angle corridors, 15px corner gaps
const Wall lv8Walls[] = {
    {0, 55, 90, 55},          // H1 from left, gap right 44px
    {90, 70, 90, 85},         // V1: 15px below H1, 15px above H2
    {40, 100, 134, 100},      // H2 from right, gap left 40px
    {40, 115, 40, 140},       // V2: 15px below H2, 15px above H3
    {0, 155, 100, 155},       // H3 from left, gap right 34px
    {100, 170, 100, 185},     // V3: 15px below H3, 15px above H4
    {50, 200, 134, 200},      // H4 from right, gap left 50px
};

// Level 9: "The Labyrinth" — dead ends, maze planning
const Wall lv9Walls[] = {
    {45, 20, 45, 70},          // V1: top divider (dead end pocket left)
    {0, 70, 110, 70},          // H1: gap right 24px
    {90, 70, 90, 115},         // V2: mid-right blocker
    {20, 130, 134, 130},       // H2: gap left 20px
    {78, 130, 78, 190},        // V3: lower-right blocker (dead end right)
    {0, 190, 45, 190},         // H3a: bottom-left shelf
    {73, 190, 134, 190},       // H3b: bottom-right shelf (gap 45-73)
    {0, 220, 30, 220},         // H4a: final gate left
    {55, 220, 134, 220},       // H4b: final gate right (gap 30-55)
};

// Level 10: "The Gauntlet" — everything combined, 14px precision gates
const Wall lv10Walls[] = {
    {0, 55, 120, 55},          // H1: gap right 14px
    {110, 55, 110, 80},        // V1: blocker near gap
    {14, 90, 134, 90},         // H2: gap left 14px
    {24, 90, 24, 115},         // V2: blocker near gap
    {0, 130, 100, 130},        // H3: gap right 34px
    {60, 130, 60, 175},        // V3: center column
    {0, 185, 55, 185},         // H4a: funnel left
    {79, 185, 134, 185},       // H4b: funnel right (gap 24px)
    {0, 222, 60, 222},         // H5a: precision gate left
    {74, 222, 134, 222},       // H5b: precision gate right (gap 14px)
};

const Level levels[] = {
    { lv1Walls,  2,  68, 35,  121, 230, 10 },  // Lv1: Figma S-curve
    { lv2Walls,  3,  68, 35,   15, 228, 10 },  // Lv2: Figma zigzag
    { lv3Walls,  4,  19, 35,   19, 223, 10 },  // Lv3: Figma corridors
    { lv4Walls,  0,  15, 35,  120, 228, 10 },  // Lv4: Moving Walls
    { lv5Walls,  0,  15, 35,  120, 228, 10 },  // Lv5: Moving Columns
    { lv6Walls,  0,  67, 55,   67, 150, 10 },  // Lv6: Rotating Arcs
    { lv7Walls,  7,  67, 35,   20, 230, 10 },  // Lv7: Spiral
    { lv8Walls,  7, 120, 35,   20, 228, 10 },  // Lv8: Corridors
    { lv9Walls,  9, 100, 35,   20, 233, 10 },  // Lv9: The Labyrinth
    { lv10Walls, 10, 125, 35,  67, 233,  8 },  // Lv10: The Gauntlet
};
const int levelCount = 6;

// Game state
enum MazePhase { MAZE_COUNTDOWN, MAZE_PLAYING, MAZE_LEVEL_CLEAR, MAZE_GAME_OVER, MAZE_ALL_CLEAR };
MazePhase mazePhase;
int currentLevel;
int hearts;
const int maxHearts = 3;

float ballX, ballY;
float ballVelX, ballVelY;
const float ballR = 4.0f;
int prevDrawX, prevDrawY;

unsigned long hitInvincibleUntil = 0;
const unsigned long HIT_INVINCIBLE_MS = 600;
unsigned long countdownStart;
int countdownStep; // 0=3, 1=2, 2=1, 3=GO

const Wall* currentWalls;
int currentWallCount;
float currentGoalX, currentGoalY;
int currentGoalSize;

// All walls for current level (borders + level walls)
Wall allWalls[14]; // max 4 borders + 10 internal

// Moving walls (levels 4 & 5)
const int MAX_MOVING_WALLS = 3;
int movingWallCount = 0;
Wall movingWalls[MAX_MOVING_WALLS];
bool movingWallVertical = false;        // false = horizontal walls moving left/right, true = vertical walls moving up/down
int movingWallFixed[MAX_MOVING_WALLS];  // fixed axis (Y for horizontal, X for vertical)
int movingWallLen = 87;                 // wall length in pixels
float movingWallPos[MAX_MOVING_WALLS];  // current position on moving axis
float movingWallSpeed = 0.5f;           // pixels per frame
int movingWallDir[MAX_MOVING_WALLS];    // per-wall direction

// Rotating arcs (level 6)
const int MAX_ARCS = 2;
int arcCount = 0;
float arcCenterX, arcCenterY;           // shared center for concentric arcs
float arcRadius[MAX_ARCS];              // radius of each arc
float arcGapAngle = 0.8f;              // gap size in radians (~45 degrees)
float arcAngle[MAX_ARCS];              // current gap center angle
float arcRotSpeed[MAX_ARCS];           // rotation speed (radians per frame)

bool collidesWithWall(float bx, float by, const Wall& w) {
    float wx1 = min(w.x1, w.x2) - 1.0f;
    float wy1 = min(w.y1, w.y2) - 1.0f;
    float wx2 = max(w.x1, w.x2) + 1.0f;
    float wy2 = max(w.y1, w.y2) + 1.0f;
    float closestX = constrain(bx, wx1, wx2);
    float closestY = constrain(by, wy1, wy2);
    float dx = bx - closestX;
    float dy = by - closestY;
    return (dx * dx + dy * dy) < (ballR * ballR);
}

bool collidesWithArc(float bx, float by, int arcIdx) {
    float dx = bx - arcCenterX;
    float dy = by - arcCenterY;
    float dist = sqrtf(dx * dx + dy * dy);
    float r = arcRadius[arcIdx];
    // Check if ball is near the arc's radius (within ballR + 2 of the circle line)
    if (fabsf(dist - r) > (ballR + 2.0f)) return false;
    // Ball is near the circle — check if it's in the gap
    float angle = atan2f(dy, dx);  // -PI to PI
    float gapCenter = arcAngle[arcIdx];
    // Normalize angle difference to -PI..PI
    float diff = angle - gapCenter;
    while (diff > M_PI) diff -= 2.0f * M_PI;
    while (diff < -M_PI) diff += 2.0f * M_PI;
    // If within gap, no collision
    if (fabsf(diff) < arcGapAngle / 2.0f) return false;
    return true;
}

bool collidesAny(float bx, float by) {
    for (int i = 0; i < currentWallCount; i++) {
        if (collidesWithWall(bx, by, allWalls[i])) return true;
    }
    for (int i = 0; i < movingWallCount; i++) {
        if (collidesWithWall(bx, by, movingWalls[i])) return true;
    }
    for (int i = 0; i < arcCount; i++) {
        if (collidesWithArc(bx, by, i)) return true;
    }
    return false;
}

// Sprite buffer for flicker-free arc rendering
static M5Canvas arcSprite(&M5.Display);
static bool arcSpriteCreated = false;

void drawArcsToSprite() {
    if (!arcSpriteCreated) {
        arcSprite.createSprite(135, 220);
        arcSpriteCreated = true;
    }
    arcSprite.fillSprite(BLACK);

    // Draw border walls on sprite
    for (int i = 0; i < currentWallCount; i++) {
        const Wall& w = allWalls[i];
        arcSprite.drawLine(w.x1, w.y1 - 20, w.x2, w.y2 - 20, WHITE);
    }

    // Draw arcs on sprite
    for (int idx = 0; idx < arcCount; idx++) {
        float r = arcRadius[idx];
        float gapCenter = arcAngle[idx];
        float halfGap = arcGapAngle / 2.0f;
        // Convert gap center to degrees (0=right, clockwise)
        float gapDeg = gapCenter * 180.0f / M_PI;
        float halfGapDeg = halfGap * 180.0f / M_PI;
        // Arc goes from (gapEnd) to (gapStart) — the solid part
        float startDeg = gapDeg + halfGapDeg;
        float endDeg = gapDeg - halfGapDeg + 360.0f;
        // drawArc: x, y, r_outer, r_inner, startAngle, endAngle, color
        int cy = (int)arcCenterY - 20; // sprite coords (no y offset)
        arcSprite.drawArc((int)arcCenterX, cy, (int)r + 1, (int)r - 1, startDeg, endDeg, WHITE);
    }

    // Draw goal flag on sprite
    int gx = (int)currentGoalX;
    int gy = (int)currentGoalY - 20;
    arcSprite.drawLine(gx, gy - 12, gx, gy + 2, WHITE);
    arcSprite.fillTriangle(gx + 1, gy - 12, gx + 1, gy - 4, gx + 8, gy - 8, GREEN);
    arcSprite.drawLine(gx - 4, gy + 2, gx + 4, gy + 2, WHITE);

    // Draw ball on sprite
    uint16_t ballColor = YELLOW;
    if (millis() < hitInvincibleUntil) {
        ballColor = ((millis() / 80) % 2 == 0) ? RED : YELLOW;
    }
    arcSprite.fillCircle(prevDrawX, prevDrawY - 20, ballR, ballColor);

    // Push sprite to display
    arcSprite.pushSprite(0, 20);
}

void drawWalls() {
    for (int i = 0; i < currentWallCount; i++) {
        const Wall& w = allWalls[i];
        M5.Display.drawLine(w.x1, w.y1, w.x2, w.y2, WHITE);
    }
    for (int i = 0; i < movingWallCount; i++) {
        const Wall& w = movingWalls[i];
        M5.Display.drawLine(w.x1, w.y1, w.x2, w.y2, WHITE);
    }
    // Arcs are drawn via sprite in drawArcsToSprite()
}

void drawGoal() {
    int gx = (int)currentGoalX;
    int gy = (int)currentGoalY;
    // Flag pole
    M5.Display.drawLine(gx, gy - 12, gx, gy + 2, WHITE);
    // Flag triangle (green)
    M5.Display.fillTriangle(gx + 1, gy - 12, gx + 1, gy - 4, gx + 8, gy - 8, GREEN);
    // Base line
    M5.Display.drawLine(gx - 4, gy + 2, gx + 4, gy + 2, WHITE);
}

void drawHUD() {
    M5.Display.fillRect(0, 0, 135, 19, BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(GREEN, BLACK);
    M5.Display.setCursor(2, 2);
    M5.Display.printf("Lv%d", currentLevel + 1);
    for (int i = 0; i < maxHearts; i++) {
        int hx = 135 - (maxHearts - i) * 15;
        uint16_t color = (i < hearts) ? RED : DARKGREY;
        M5.Display.fillRect(hx, 4, 11, 11, color);
    }
}

void loadLevel() {
    const Level& lv = levels[currentLevel];
    // Copy borders + level walls into allWalls
    for (int i = 0; i < borderCount; i++) allWalls[i] = borderWalls[i];
    for (int i = 0; i < lv.wallCount; i++) allWalls[borderCount + i] = lv.walls[i];
    currentWallCount = borderCount + lv.wallCount;
    currentGoalX = lv.goalX;
    currentGoalY = lv.goalY;
    currentGoalSize = lv.goalSize;
    ballX = lv.startX;
    ballY = lv.startY;
    ballVelX = ballVelY = 0;
    prevDrawX = (int)ballX;
    prevDrawY = (int)ballY;
    hitInvincibleUntil = 0;

    // Set up moving walls
    if (currentLevel == 3) {
        // Level 4: two horizontal walls moving left/right
        movingWallVertical = false;
        movingWallCount = 2;
        movingWallLen = 87;  // 135 - 48px gap
        movingWallFixed[0] = 93;   // Y positions
        movingWallFixed[1] = 166;
        movingWallPos[0] = 0;                      // start left
        movingWallPos[1] = 135 - movingWallLen;    // start right
        movingWallDir[0] = 1;
        movingWallDir[1] = -1;
        movingWallSpeed = 0.5f;
    } else if (currentLevel == 4) {
        // Level 5: three vertical walls moving up/down
        movingWallVertical = true;
        movingWallCount = 3;
        movingWallLen = 110;  // half of 220px maze height
        movingWallFixed[0] = 31;   // X positions
        movingWallFixed[1] = 68;
        movingWallFixed[2] = 105;
        movingWallPos[0] = 239 - movingWallLen;  // start bottom
        movingWallPos[1] = 20;                    // start top
        movingWallPos[2] = 239 - movingWallLen;  // start bottom
        movingWallDir[0] = -1;   // moving up
        movingWallDir[1] = 1;    // moving down
        movingWallDir[2] = -1;   // moving up
        movingWallSpeed = 0.5f;
    } else {
        movingWallCount = 0;
    }

    // Set up rotating arcs for level 6 (index 5)
    if (currentLevel == 5) {
        arcCount = 2;
        arcCenterX = 67;           // center of screen
        arcCenterY = 150;          // center of maze area
        arcRadius[0] = 55;         // large outer arc
        arcRadius[1] = 25;         // small inner arc
        arcAngle[0] = 0;           // gap starts at right
        arcAngle[1] = M_PI;        // gap starts at left
        arcRotSpeed[0] = 0.03f;    // clockwise
        arcRotSpeed[1] = -0.03f;   // counter-clockwise
        arcGapAngle = 1.6f;        // ~90 degrees gap
    } else {
        arcCount = 0;
    }

    // Initialize moving wall structs
    for (int i = 0; i < movingWallCount; i++) {
        int p = (int)movingWallPos[i];
        int f = movingWallFixed[i];
        if (movingWallVertical) {
            movingWalls[i] = {(int16_t)f, (int16_t)p, (int16_t)f, (int16_t)(p + movingWallLen)};
        } else {
            movingWalls[i] = {(int16_t)p, (int16_t)f, (int16_t)(p + movingWallLen), (int16_t)f};
        }
    }
}

void drawMazeScreen() {
    M5.Display.fillScreen(BLACK);
    drawHUD();
    if (arcCount > 0) {
        drawArcsToSprite();
    } else {
        drawWalls();
        drawGoal();
        M5.Display.fillCircle(prevDrawX, prevDrawY, ballR, YELLOW);
    }
}

void startCountdown() {
    mazePhase = MAZE_COUNTDOWN;
    countdownStep = 0;
    countdownStart = millis();
    drawMazeScreen();
}

void enterMaze() {
    currentLevel = 0;
    hearts = maxHearts;
    loadLevel();
    startCountdown();
}

void showGameOver() {
    mazePhase = MAZE_GAME_OVER;
    M5.Display.fillScreen(RED);
    delay(200);
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextColor(RED, BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(8, 50);
    M5.Display.print("GAME OVER");
    M5.Display.setTextColor(WHITE, BLACK);
    M5.Display.setCursor(8, 90);
    M5.Display.printf("Level %d", currentLevel + 1);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(WHITE, BLACK);
    drawRetryBackHint(WHITE);
    M5.Speaker.tone(400, 150);
    delay(160);
    M5.Speaker.tone(300, 150);
    delay(160);
    M5.Speaker.tone(200, 300);
}

void showLevelClear() {
    mazePhase = MAZE_LEVEL_CLEAR;
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextColor(GREEN, BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(8, 50);
    M5.Display.printf("Level %d", currentLevel + 1);
    M5.Display.setCursor(8, 75);
    M5.Display.print("Clear!");
    // Show remaining hearts
    for (int i = 0; i < maxHearts; i++) {
        int hx = 8 + i * 15;
        uint16_t color = (i < hearts) ? RED : DARKGREY;
        M5.Display.fillRect(hx, 110, 11, 11, color);
    }
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(WHITE, BLACK);
    M5.Display.setCursor(4, 210);
    M5.Display.print("A:Next B:");
    drawLeftArrow(WHITE);
    M5.Speaker.tone(523, 100);
    delay(110);
    M5.Speaker.tone(659, 100);
    delay(110);
    M5.Speaker.tone(784, 200);
}

// Fireworks state
struct Spark {
    float x, y, vx, vy;
    uint16_t color;
    int life;
};
const int MAX_SPARKS = 30;
Spark sparks[MAX_SPARKS];
int sparkCount = 0;
unsigned long lastFirework = 0;
unsigned long lastFireworkSound = 0;

void spawnFirework() {
    float cx = random(20, 115);
    float cy = random(30, 140);
    uint16_t colors[] = {RED, YELLOW, GREEN, CYAN, 0xF81F, WHITE};
    uint16_t color = colors[random(0, 6)];
    int count = random(8, 15);
    for (int i = 0; i < count && sparkCount < MAX_SPARKS; i++) {
        float angle = random(0, 628) / 100.0f;
        float speed = random(5, 20) / 10.0f;
        sparks[sparkCount] = {cx, cy, speed * cosf(angle), speed * sinf(angle), color, random(15, 30)};
        sparkCount++;
    }
}

void drawTrophy() {
    // Cup body
    M5.Display.fillRect(45, 55, 45, 35, YELLOW);
    // Cup rim
    M5.Display.fillRect(40, 50, 55, 8, YELLOW);
    // Handles
    M5.Display.drawArc(45, 68, 12, 8, 180, 270, YELLOW);
    M5.Display.drawArc(90, 68, 12, 8, 270, 360, YELLOW);
    // Stem
    M5.Display.fillRect(62, 90, 11, 15, YELLOW);
    // Base
    M5.Display.fillRect(50, 105, 35, 6, YELLOW);
    // Star on cup
    M5.Display.setTextColor(BLACK, YELLOW);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(60, 62);
    M5.Display.print("*");
}

void showAllClear() {
    mazePhase = MAZE_ALL_CLEAR;
    M5.Display.fillScreen(BLACK);

    drawTrophy();

    M5.Display.setTextColor(YELLOW, BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(14, 125);
    M5.Display.print("CHAMPION!");
    M5.Display.setTextColor(WHITE, BLACK);
    M5.Display.setTextSize(2);
    drawRetryBackHint(WHITE);

    // Celebratory fanfare
    M5.Speaker.tone(523, 100); delay(110);
    M5.Speaker.tone(659, 100); delay(110);
    M5.Speaker.tone(784, 100); delay(110);
    M5.Speaker.tone(1047, 150); delay(160);
    M5.Speaker.tone(784, 100); delay(110);
    M5.Speaker.tone(1047, 400);

    sparkCount = 0;
    lastFirework = millis();
    lastFireworkSound = millis();
}

void updateMaze() {
    if (M5.BtnB.wasPressed() && mazePhase != MAZE_COUNTDOWN) {
        switchState(STATE_MENU);
        return;
    }

    switch (mazePhase) {
        case MAZE_COUNTDOWN: {
            unsigned long elapsed = millis() - countdownStart;
            int step = elapsed / 700;
            if (step > 3) step = 3;
            if (step != countdownStep) {
                countdownStep = step;
                // Erase previous countdown text
                M5.Display.fillRect(30, 100, 80, 40, BLACK);
                // Redraw walls/goal that may have been covered
                drawWalls();
                drawGoal();
                M5.Display.setTextSize(3);
                M5.Display.setTextColor(WHITE, BLACK);
                if (step < 3) {
                    char num = '3' - step;
                    M5.Display.setCursor(58, 105);
                    M5.Display.print(num);
                    M5.Speaker.tone(523, 50);
                } else {
                    M5.Display.setCursor(34, 105);
                    M5.Display.print("GO!");
                    M5.Speaker.tone(1047, 80);
                }
            }
            if (elapsed >= 3200) {
                // Clear countdown text and start playing
                M5.Display.fillRect(30, 100, 80, 40, BLACK);
                drawWalls();
                drawGoal();
                M5.Display.fillCircle(prevDrawX, prevDrawY, ballR, YELLOW);
                mazePhase = MAZE_PLAYING;
            }
            break;
        }

        case MAZE_PLAYING: {
            // Update moving walls
            if (movingWallCount > 0) {
                // Erase old positions
                for (int i = 0; i < movingWallCount; i++) {
                    const Wall& w = movingWalls[i];
                    M5.Display.drawLine(w.x1, w.y1, w.x2, w.y2, BLACK);
                }
                // Move walls
                float maxPos = movingWallVertical ? (239 - movingWallLen) : (135 - movingWallLen);
                float minPos = movingWallVertical ? 20 : 0;
                for (int i = 0; i < movingWallCount; i++) {
                    movingWallPos[i] += movingWallSpeed * movingWallDir[i];
                    if (movingWallPos[i] >= maxPos) { movingWallPos[i] = maxPos; movingWallDir[i] = -1; }
                    if (movingWallPos[i] <= minPos) { movingWallPos[i] = minPos; movingWallDir[i] = 1; }
                    int p = (int)movingWallPos[i];
                    int f = movingWallFixed[i];
                    if (movingWallVertical) {
                        movingWalls[i] = {(int16_t)f, (int16_t)p, (int16_t)f, (int16_t)(p + movingWallLen)};
                    } else {
                        movingWalls[i] = {(int16_t)p, (int16_t)f, (int16_t)(p + movingWallLen), (int16_t)f};
                    }
                }
            }

            // Update rotating arcs
            if (arcCount > 0) {
                for (int i = 0; i < arcCount; i++) {
                    arcAngle[i] += arcRotSpeed[i];
                    if (arcAngle[i] > M_PI) arcAngle[i] -= 2.0f * M_PI;
                    if (arcAngle[i] < -M_PI) arcAngle[i] += 2.0f * M_PI;
                }
            }

            M5.Imu.update();
            auto imu = M5.Imu.getImuData();

            ballVelX -= imu.accel.x * 0.4f;
            ballVelY += imu.accel.y * 0.4f;
            ballVelX *= 0.88f;
            ballVelY *= 0.88f;

            float testX = ballX + ballVelX;
            bool hitX = collidesAny(testX, ballY);
            if (hitX) { ballVelX = 0; } else { ballX = testX; }

            float testY = ballY + ballVelY;
            bool hitY = collidesAny(ballX, testY);
            if (hitY) { ballVelY = 0; } else { ballY = testY; }

            // Wall hit damage
            if ((hitX || hitY) && millis() > hitInvincibleUntil) {
                hearts--;
                hitInvincibleUntil = millis() + HIT_INVINCIBLE_MS;
                ballVelX = ballVelY = 0;
                M5.Speaker.tone(150, 80);
                drawHUD();
                if (hearts <= 0) {
                    showGameOver();
                    return;
                }
            }

            // Draw ball
            int drawX = (int)ballX;
            int drawY = (int)ballY;

            if (arcCount > 0) {
                prevDrawX = drawX;
                prevDrawY = drawY;
                // Use sprite buffer for flicker-free arc rendering
                drawArcsToSprite();
            } else {
                bool moved = (drawX != prevDrawX || drawY != prevDrawY);
                if (moved || movingWallCount > 0) {
                    M5.Display.fillCircle(prevDrawX, prevDrawY, ballR + 1, BLACK);
                    drawWalls();
                    drawGoal();
                    prevDrawX = drawX;
                    prevDrawY = drawY;
                }
                // Ball color (blink during invincibility)
                uint16_t ballColor = YELLOW;
                if (millis() < hitInvincibleUntil) {
                    ballColor = ((millis() / 80) % 2 == 0) ? RED : YELLOW;
                }
                M5.Display.fillCircle(prevDrawX, prevDrawY, ballR, ballColor);
            }

            // Win check
            if (abs(ballX - currentGoalX) < (currentGoalSize / 2 + ballR) &&
                abs(ballY - currentGoalY) < (currentGoalSize / 2 + ballR)) {
                if (currentLevel >= levelCount - 1) {
                    showAllClear();
                } else {
                    showLevelClear();
                }
            }
            break;
        }

        case MAZE_LEVEL_CLEAR:
            if (M5.BtnA.wasPressed()) {
                currentLevel++;
                loadLevel();
                startCountdown();
            }
            break;

        case MAZE_GAME_OVER:
            if (M5.BtnA.wasPressed()) {
                hearts = maxHearts;
                loadLevel();
                startCountdown();
            }
            break;

        case MAZE_ALL_CLEAR: {
            if (M5.BtnA.wasPressed()) {
                currentLevel = 0;
                hearts = maxHearts;
                loadLevel();
                startCountdown();
                break;
            }
            // Animate fireworks
            unsigned long now = millis();
            // Spawn new firework every 500ms
            if (now - lastFirework > 500) {
                lastFirework = now;
                spawnFirework();
            }
            // Firework pop sound every 800ms
            if (now - lastFireworkSound > 800) {
                lastFireworkSound = now;
                M5.Speaker.tone(random(800, 1500), 50);
            }
            // Update and draw sparks
            for (int i = 0; i < sparkCount; i++) {
                // Erase old position
                M5.Display.drawPixel((int)sparks[i].x, (int)sparks[i].y, BLACK);
                // Update
                sparks[i].x += sparks[i].vx;
                sparks[i].y += sparks[i].vy;
                sparks[i].vy += 0.05f; // gravity
                sparks[i].life--;
                // Draw new position or remove
                if (sparks[i].life <= 0 || sparks[i].y > 240 || sparks[i].x < 0 || sparks[i].x > 135) {
                    sparks[i] = sparks[sparkCount - 1];
                    sparkCount--;
                    i--;
                } else {
                    M5.Display.drawPixel((int)sparks[i].x, (int)sparks[i].y, sparks[i].color);
                }
            }
            break;
        }
    }
}

// --- Main ---

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Display.setRotation(0);
    M5.Display.setBrightness(80);
    M5.Speaker.setVolume(255);
    Serial.begin(115200);
    randomSeed(analogRead(0) ^ millis());
    Serial.println("Boot OK");
    enterMenu();
}

void loop() {
    M5.update();

    switch (currentState) {
        case STATE_MENU:       updateMenu();     break;
        case STATE_REACTION:   updateReaction(); break;
        case STATE_MAGIC8BALL: updateMagic8();   break;
        case STATE_TILTMAZE:   updateMaze();     break;
    }

    delay(10);
}
