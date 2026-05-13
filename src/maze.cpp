#include "maze.h"

// --- Structs ---

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

// --- Level Data ---

static const Wall borderWalls[] = {
    {0, 20, 134, 20},     // top
    {0, 20, 0, 239},      // left
    {134, 20, 134, 239},  // right
    {0, 239, 134, 239},   // bottom
};
static const int borderCount = 4;

static const Wall lv1Walls[] = {
    {0, 93, 103, 93},
    {32, 166, 134, 166},
};

static const Wall lv2Walls[] = {
    {0, 75, 103, 75},
    {32, 130, 134, 130},
    {0, 185, 103, 185},
};

static const Wall lv3Walls[] = {
    {45, 20, 45, 43},
    {0, 91, 90, 91},
    {45, 139, 45, 239},
    {90, 60, 90, 191},
};

static const Wall lv4Walls[] = {};
static const Wall lv5Walls[] = {};
static const Wall lv6Walls[] = {};

static const Wall lv7Walls[] = {
    {0, 60, 100, 60},
    {100, 75, 100, 105},
    {34, 120, 134, 120},
    {34, 135, 34, 165},
    {0, 180, 100, 180},
    {100, 195, 100, 205},
    {50, 220, 134, 220},
};

static const Wall lv8Walls[] = {
    {0, 55, 90, 55},
    {90, 70, 90, 85},
    {40, 100, 134, 100},
    {40, 115, 40, 140},
    {0, 155, 100, 155},
    {100, 170, 100, 185},
    {50, 200, 134, 200},
};

static const Wall lv9Walls[] = {
    {45, 20, 45, 70},
    {0, 70, 110, 70},
    {90, 70, 90, 115},
    {20, 130, 134, 130},
    {78, 130, 78, 190},
    {0, 190, 45, 190},
    {73, 190, 134, 190},
    {0, 220, 30, 220},
    {55, 220, 134, 220},
};

static const Wall lv10Walls[] = {
    {0, 55, 120, 55},
    {110, 55, 110, 80},
    {14, 90, 134, 90},
    {24, 90, 24, 115},
    {0, 130, 100, 130},
    {60, 130, 60, 175},
    {0, 185, 55, 185},
    {79, 185, 134, 185},
    {0, 222, 60, 222},
    {74, 222, 134, 222},
};

static const Level levels[] = {
    { lv1Walls,  2,  68, 35,  121, 230, 10 },
    { lv2Walls,  3,  68, 35,   15, 228, 10 },
    { lv3Walls,  4,  19, 35,   19, 223, 10 },
    { lv4Walls,  0,  15, 35,  120, 228, 10 },
    { lv5Walls,  0,  15, 35,  120, 228, 10 },
    { lv6Walls,  0,  67, 55,   67, 150, 10 },
    { lv7Walls,  7,  67, 35,   20, 230, 10 },
    { lv8Walls,  7, 120, 35,   20, 228, 10 },
    { lv9Walls,  9, 100, 35,   20, 233, 10 },
    { lv10Walls, 10, 125, 35,  67, 233,  8 },
};
static const int levelCount = 6;

// --- Game State ---

static enum MazePhase { MAZE_COUNTDOWN, MAZE_PLAYING, MAZE_LEVEL_CLEAR, MAZE_GAME_OVER, MAZE_ALL_CLEAR } mazePhase;
static int currentLevel;
static int hearts;
static const int maxHearts = 3;

static float ballX, ballY;
static float ballVelX, ballVelY;
static const float ballR = 4.0f;
static int prevDrawX, prevDrawY;

static unsigned long hitInvincibleUntil = 0;
static const unsigned long HIT_INVINCIBLE_MS = 600;
static unsigned long countdownStart;
static int countdownStep;

static float currentGoalX, currentGoalY;
static int currentGoalSize;

static Wall allWalls[14];
static int currentWallCount;

// Moving walls
static const int MAX_MOVING_WALLS = 3;
static int movingWallCount = 0;
static Wall movingWalls[MAX_MOVING_WALLS];
static bool movingWallVertical = false;
static int movingWallFixed[MAX_MOVING_WALLS];
static int movingWallLen = 87;
static float movingWallPos[MAX_MOVING_WALLS];
static float movingWallSpeed = 0.5f;
static int movingWallDir[MAX_MOVING_WALLS];

// Rotating arcs
static const int MAX_ARCS = 2;
static int arcCount = 0;
static float arcCenterX, arcCenterY;
static float arcRadius[MAX_ARCS];
static float arcGapAngle = 0.8f;
static float arcAngle[MAX_ARCS];
static float arcRotSpeed[MAX_ARCS];

// Fireworks
struct Spark {
    float x, y, vx, vy;
    uint16_t color;
    int life;
};
static const int MAX_SPARKS = 30;
static Spark sparks[MAX_SPARKS];
static int sparkCount = 0;
static unsigned long lastFirework = 0;
static unsigned long lastFireworkSound = 0;

// Sprite buffer for arc rendering
static M5Canvas arcSprite(&M5.Display);
static bool arcSpriteCreated = false;

// --- Collision ---

static bool collidesWithWall(float bx, float by, const Wall& w) {
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

static bool collidesWithArc(float bx, float by, int arcIdx) {
    float dx = bx - arcCenterX;
    float dy = by - arcCenterY;
    float dist = sqrtf(dx * dx + dy * dy);
    float r = arcRadius[arcIdx];
    if (fabsf(dist - r) > (ballR + 2.0f)) return false;
    float angle = atan2f(dy, dx);
    float gapCenter = arcAngle[arcIdx];
    float diff = angle - gapCenter;
    while (diff > M_PI) diff -= 2.0f * M_PI;
    while (diff < -M_PI) diff += 2.0f * M_PI;
    if (fabsf(diff) < arcGapAngle / 2.0f) return false;
    return true;
}

static bool collidesAny(float bx, float by) {
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

// --- Drawing ---

static void drawArcsToSprite() {
    if (!arcSpriteCreated) {
        arcSprite.createSprite(135, 220);
        arcSpriteCreated = true;
    }
    arcSprite.fillSprite(BLACK);
    for (int i = 0; i < currentWallCount; i++) {
        const Wall& w = allWalls[i];
        arcSprite.drawLine(w.x1, w.y1 - 20, w.x2, w.y2 - 20, WHITE);
    }
    for (int idx = 0; idx < arcCount; idx++) {
        float r = arcRadius[idx];
        float gapCenter = arcAngle[idx];
        float halfGap = arcGapAngle / 2.0f;
        float gapDeg = gapCenter * 180.0f / M_PI;
        float halfGapDeg = halfGap * 180.0f / M_PI;
        float startDeg = gapDeg + halfGapDeg;
        float endDeg = gapDeg - halfGapDeg + 360.0f;
        int cy = (int)arcCenterY - 20;
        arcSprite.drawArc((int)arcCenterX, cy, (int)r + 1, (int)r - 1, startDeg, endDeg, WHITE);
    }
    int gx = (int)currentGoalX;
    int gy = (int)currentGoalY - 20;
    arcSprite.drawLine(gx, gy - 12, gx, gy + 2, WHITE);
    arcSprite.fillTriangle(gx + 1, gy - 12, gx + 1, gy - 4, gx + 8, gy - 8, GREEN);
    arcSprite.drawLine(gx - 4, gy + 2, gx + 4, gy + 2, WHITE);
    uint16_t ballColor = YELLOW;
    if (millis() < hitInvincibleUntil) {
        ballColor = ((millis() / 80) % 2 == 0) ? RED : YELLOW;
    }
    arcSprite.fillCircle(prevDrawX, prevDrawY - 20, ballR, ballColor);
    arcSprite.pushSprite(0, 20);
}

static void drawWalls() {
    for (int i = 0; i < currentWallCount; i++) {
        const Wall& w = allWalls[i];
        M5.Display.drawLine(w.x1, w.y1, w.x2, w.y2, WHITE);
    }
    for (int i = 0; i < movingWallCount; i++) {
        const Wall& w = movingWalls[i];
        M5.Display.drawLine(w.x1, w.y1, w.x2, w.y2, WHITE);
    }
}

static void drawGoal() {
    int gx = (int)currentGoalX;
    int gy = (int)currentGoalY;
    M5.Display.drawLine(gx, gy - 12, gx, gy + 2, WHITE);
    M5.Display.fillTriangle(gx + 1, gy - 12, gx + 1, gy - 4, gx + 8, gy - 8, GREEN);
    M5.Display.drawLine(gx - 4, gy + 2, gx + 4, gy + 2, WHITE);
}

static void drawHUD() {
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

// --- Level Loading ---

static void loadLevel() {
    const Level& lv = levels[currentLevel];
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

    if (currentLevel == 3) {
        movingWallVertical = false;
        movingWallCount = 2;
        movingWallLen = 87;
        movingWallFixed[0] = 93;
        movingWallFixed[1] = 166;
        movingWallPos[0] = 0;
        movingWallPos[1] = 135 - movingWallLen;
        movingWallDir[0] = 1;
        movingWallDir[1] = -1;
        movingWallSpeed = 0.5f;
    } else if (currentLevel == 4) {
        movingWallVertical = true;
        movingWallCount = 3;
        movingWallLen = 110;
        movingWallFixed[0] = 31;
        movingWallFixed[1] = 68;
        movingWallFixed[2] = 105;
        movingWallPos[0] = 239 - movingWallLen;
        movingWallPos[1] = 20;
        movingWallPos[2] = 239 - movingWallLen;
        movingWallDir[0] = -1;
        movingWallDir[1] = 1;
        movingWallDir[2] = -1;
        movingWallSpeed = 0.5f;
    } else {
        movingWallCount = 0;
    }

    if (currentLevel == 5) {
        arcCount = 2;
        arcCenterX = 67;
        arcCenterY = 150;
        arcRadius[0] = 55;
        arcRadius[1] = 25;
        arcAngle[0] = 0;
        arcAngle[1] = M_PI;
        arcRotSpeed[0] = 0.03f;
        arcRotSpeed[1] = -0.03f;
        arcGapAngle = 1.6f;
    } else {
        arcCount = 0;
    }

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

// --- Screens ---

static void drawMazeScreen() {
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

static void startCountdown() {
    mazePhase = MAZE_COUNTDOWN;
    countdownStep = 0;
    countdownStart = millis();
    drawMazeScreen();
}

static void showGameOver() {
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

static void showLevelClear() {
    mazePhase = MAZE_LEVEL_CLEAR;
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextColor(GREEN, BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(8, 50);
    M5.Display.printf("Level %d", currentLevel + 1);
    M5.Display.setCursor(8, 75);
    M5.Display.print("Clear!");
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

// Trophy bounding box (used to protect trophy from firework erase)
static const int TROPHY_X1 = 32, TROPHY_Y1 = 49, TROPHY_X2 = 103, TROPHY_Y2 = 112;

static bool insideTrophy(int x, int y) {
    return x >= TROPHY_X1 && x <= TROPHY_X2 && y >= TROPHY_Y1 && y <= TROPHY_Y2;
}

static void spawnFirework() {
    float cx, cy;
    // Spawn fireworks outside the trophy area
    if (random(0, 2) == 0) {
        cx = random(10, 125);
        cy = random(140, 200);
    } else {
        cx = random(0, 2) == 0 ? random(5, TROPHY_X1) : random(TROPHY_X2, 130);
        cy = random(30, 120);
    }
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

static void drawTrophy() {
    M5.Display.fillRect(45, 55, 45, 35, YELLOW);
    M5.Display.fillRect(40, 50, 55, 8, YELLOW);
    M5.Display.drawArc(45, 68, 12, 8, 180, 270, YELLOW);
    M5.Display.drawArc(90, 68, 12, 8, 270, 360, YELLOW);
    M5.Display.fillRect(62, 90, 11, 15, YELLOW);
    M5.Display.fillRect(50, 105, 35, 6, YELLOW);
    M5.Display.setTextColor(BLACK, YELLOW);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(60, 62);
    M5.Display.print("*");
}

static void showAllClear() {
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

// --- Public Interface ---

void enterMaze() {
    currentLevel = 0;
    hearts = maxHearts;
    loadLevel();
    startCountdown();
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
                M5.Display.fillRect(30, 100, 80, 40, BLACK);
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
                M5.Display.fillRect(30, 100, 80, 40, BLACK);
                drawWalls();
                drawGoal();
                M5.Display.fillCircle(prevDrawX, prevDrawY, ballR, YELLOW);
                mazePhase = MAZE_PLAYING;
            }
            break;
        }

        case MAZE_PLAYING: {
            if (movingWallCount > 0) {
                for (int i = 0; i < movingWallCount; i++) {
                    const Wall& w = movingWalls[i];
                    M5.Display.drawLine(w.x1, w.y1, w.x2, w.y2, BLACK);
                }
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

            int drawX = (int)ballX;
            int drawY = (int)ballY;

            if (arcCount > 0) {
                prevDrawX = drawX;
                prevDrawY = drawY;
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
                uint16_t ballColor = YELLOW;
                if (millis() < hitInvincibleUntil) {
                    ballColor = ((millis() / 80) % 2 == 0) ? RED : YELLOW;
                }
                M5.Display.fillCircle(prevDrawX, prevDrawY, ballR, ballColor);
            }

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
            unsigned long now = millis();
            if (now - lastFirework > 500) {
                lastFirework = now;
                spawnFirework();
            }
            if (now - lastFireworkSound > 800) {
                lastFireworkSound = now;
                M5.Speaker.tone(random(800, 1500), 50);
            }
            for (int i = 0; i < sparkCount; i++) {
                int oldX = (int)sparks[i].x;
                int oldY = (int)sparks[i].y;
                if (!insideTrophy(oldX, oldY)) {
                    M5.Display.drawPixel(oldX, oldY, BLACK);
                }
                sparks[i].x += sparks[i].vx;
                sparks[i].y += sparks[i].vy;
                sparks[i].vy += 0.05f;
                sparks[i].life--;
                if (sparks[i].life <= 0 || sparks[i].y > 240 || sparks[i].x < 0 || sparks[i].x > 135) {
                    sparks[i] = sparks[sparkCount - 1];
                    sparkCount--;
                    i--;
                } else {
                    int newX = (int)sparks[i].x;
                    int newY = (int)sparks[i].y;
                    if (!insideTrophy(newX, newY)) {
                        M5.Display.drawPixel(newX, newY, sparks[i].color);
                    }
                }
            }
            break;
        }
    }
}
