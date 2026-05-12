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
const char* menuItems[] = { "Reaction", "Magic 8-Ball", "Tilt Maze" };
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
    M5.Display.print("Pick a Game!");
    M5.Display.drawLine(0, 35, 135, 35, GREEN);
    drawMenuItems();
    // Navigation hint at bottom
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(DARKGREY, BLACK);
    M5.Display.setCursor(4, 220);
    M5.Display.print("A:scroll  B:select");
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
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(DARKGREY, BLACK);
    M5.Display.setCursor(4, 220);
    M5.Display.print("B: back to menu");
}

// --- Reaction Timer ---

enum ReactionPhase { RT_WAITING, RT_GO, RT_RESULT, RT_TOO_EARLY };
ReactionPhase rtPhase;
unsigned long rtGoTime;
unsigned long rtDelay;
unsigned long rtWaitStart;
unsigned long rtReactionMs;

void startReactionRound() {
    rtPhase = RT_WAITING;
    rtDelay = random(2000, 5001);
    rtWaitStart = millis();
    M5.Display.fillScreen(RED);
    M5.Display.setTextColor(WHITE, RED);
    M5.Display.setTextSize(3);
    M5.Display.setCursor(12, 90);
    M5.Display.print("Wait...");
    M5.Display.setTextSize(1);
    M5.Display.setCursor(4, 220);
    M5.Display.print("B: back to menu");
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
                M5.Display.setTextSize(1);
                M5.Display.setCursor(8, 120);
                M5.Display.print("A: try again");
                M5.Display.setCursor(8, 140);
                M5.Display.print("B: menu");
            } else if (millis() - rtWaitStart >= rtDelay) {
                rtPhase = RT_GO;
                rtGoTime = millis();
                M5.Display.fillScreen(GREEN);
                M5.Display.setTextColor(BLACK, GREEN);
                M5.Display.setTextSize(4);
                M5.Display.setCursor(30, 85);
                M5.Display.print("GO!");
                M5.Speaker.tone(1500, 80);
            }
            break;

        case RT_GO:
            if (M5.BtnA.wasPressed()) {
                rtPhase = RT_RESULT;
                rtReactionMs = millis() - rtGoTime;
                M5.Display.fillScreen(BLACK);
                M5.Display.setTextColor(WHITE, BLACK);
                M5.Display.setTextSize(3);
                M5.Display.setCursor(8, 40);
                M5.Display.printf("%lu ms", rtReactionMs);
                // Rating
                M5.Display.setTextSize(2);
                M5.Display.setCursor(8, 90);
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
                M5.Display.setTextSize(1);
                M5.Display.setTextColor(DARKGREY, BLACK);
                M5.Display.setCursor(8, 160);
                M5.Display.print("A: play again");
                M5.Display.setCursor(8, 175);
                M5.Display.print("B: menu");
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
    "Yes!", "No.", "Maybe...", "Definitely!",
    "Ask again", "No way!", "For sure!",
    "Not likely", "Absolutely!", "Doubtful",
    "100% yes!", "Nope!", "Signs say yes",
    "Better not", "Go for it!", "Not now",
    "Of course!", "Who knows?", "Oh yeah!", "Hmm no"
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
    M5.Display.fillScreen(TFT_NAVY);
    // Draw 8-ball circle
    M5.Display.fillCircle(67, 100, 50, BLACK);
    M5.Display.drawCircle(67, 100, 50, WHITE);
    M5.Display.drawCircle(67, 100, 49, WHITE);
    // Inner blue triangle area
    M5.Display.fillTriangle(47, 82, 87, 82, 67, 118, TFT_NAVY);
    // "8" on the ball
    M5.Display.setTextColor(WHITE, BLACK);
    M5.Display.setTextSize(3);
    M5.Display.setCursor(57, 86);
    M5.Display.print("8");
    // Prompt
    M5.Display.setTextColor(WHITE, TFT_NAVY);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(12, 175);
    M5.Display.print("Shake me!");
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(DARKGREY, TFT_NAVY);
    M5.Display.setCursor(4, 220);
    M5.Display.print("B: back to menu");
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
                M5.Display.fillScreen(TFT_NAVY);
                M5.Display.setTextColor(WHITE, TFT_NAVY);
                M5.Display.setTextSize(2);
                M5.Display.setCursor(8, 100);
                M5.Display.print("Thinking");
                M5.Speaker.tone(800, 50);
            }
            break;

        case M8_THINKING: {
            // Animate dots
            unsigned long now = millis();
            if (now - m8LastDot > 300) {
                m8LastDot = now;
                m8DotCount = (m8DotCount % 3) + 1;
                M5.Display.setCursor(104, 100);
                M5.Display.setTextColor(WHITE, TFT_NAVY);
                M5.Display.setTextSize(2);
                for (int i = 0; i < 3; i++) {
                    M5.Display.print(i < m8DotCount ? "." : " ");
                }
            }
            // After 1.5s, show answer
            if (now - m8ThinkStart >= 1500) {
                int idx = random(0, magic8Count);
                m8Phase = M8_ANSWER;
                m8AnswerTime = now;
                M5.Display.fillScreen(TFT_NAVY);
                // Draw answer ball
                M5.Display.fillCircle(67, 95, 55, BLACK);
                M5.Display.drawCircle(67, 95, 55, WHITE);
                M5.Display.drawCircle(67, 95, 54, WHITE);
                // Blue triangle window
                M5.Display.fillTriangle(30, 72, 104, 72, 67, 122, TFT_NAVY);
                // Answer text centered in triangle
                M5.Display.setTextColor(WHITE, TFT_NAVY);
                const char* answer = magic8Answers[idx];
                int len = strlen(answer);
                if (len > 10) {
                    M5.Display.setTextSize(1);
                    M5.Display.setCursor(67 - len * 3, 92);
                } else {
                    M5.Display.setTextSize(2);
                    M5.Display.setCursor(67 - len * 6, 88);
                }
                M5.Display.print(answer);
                // Instructions
                M5.Display.setTextSize(1);
                M5.Display.setTextColor(DARKGREY, TFT_NAVY);
                M5.Display.setCursor(4, 175);
                M5.Display.print("Shake for another!");
                M5.Display.setCursor(4, 220);
                M5.Display.print("B: back to menu");
                M5.Speaker.tone(500, 100);
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
                M5.Display.fillScreen(TFT_NAVY);
                M5.Display.setTextColor(WHITE, TFT_NAVY);
                M5.Display.setTextSize(2);
                M5.Display.setCursor(8, 100);
                M5.Display.print("Thinking");
                M5.Speaker.tone(800, 50);
            }
            break;
    }
}

// --- Tilt Maze ---

struct Wall {
    int16_t x1, y1, x2, y2;
};

// Screen is 135x240, maze area starts at y=20
// S-shaped zigzag: walls alternate from left and right
const Wall mazeWalls[] = {
    // Borders
    {0, 20, 134, 20},       // top
    {0, 20, 0, 239},        // left
    {134, 20, 134, 239},    // right
    {0, 239, 134, 239},     // bottom
    // Internal walls creating zigzag path
    {0, 70, 100, 70},       // from left
    {34, 120, 134, 120},    // from right
    {0, 170, 100, 170},     // from left
    {34, 220, 134, 220},    // from right
};
const int wallCount = 8;

float ballX, ballY;
float ballVelX, ballVelY;
const float ballR = 4.0f;
int prevDrawX, prevDrawY;
bool mazeWon;

// Goal position (bottom-right area, between last two walls)
const int goalX = 15;
const int goalY = 230;
const int goalSize = 12;

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

bool collidesAny(float bx, float by) {
    for (int i = 0; i < wallCount; i++) {
        if (collidesWithWall(bx, by, mazeWalls[i])) return true;
    }
    return false;
}

void drawWalls() {
    for (int i = 0; i < wallCount; i++) {
        const Wall& w = mazeWalls[i];
        M5.Display.drawLine(w.x1, w.y1, w.x2, w.y2, WHITE);
    }
}

void drawGoal() {
    M5.Display.fillRect(goalX - goalSize / 2, goalY - goalSize / 2,
                        goalSize, goalSize, GREEN);
}

void resetMaze() {
    ballX = 115.0f;
    ballY = 40.0f;
    ballVelX = ballVelY = 0;
    prevDrawX = (int)ballX;
    prevDrawY = (int)ballY;
    mazeWon = false;
}

void enterMaze() {
    resetMaze();
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(GREEN, BLACK);
    M5.Display.setCursor(4, 5);
    M5.Display.print("Tilt Maze  B:menu");
    drawWalls();
    drawGoal();
    M5.Display.fillCircle(prevDrawX, prevDrawY, ballR, YELLOW);
}

void updateMaze() {
    if (M5.BtnB.wasPressed()) {
        switchState(STATE_MENU);
        return;
    }

    if (mazeWon) {
        if (M5.BtnA.wasPressed()) {
            enterMaze();
        }
        return;
    }

    M5.Imu.update();
    auto imu = M5.Imu.getImuData();

    // Apply tilt as acceleration (signs may need flipping - test on device)
    ballVelX -= imu.accel.x * 0.4f;
    ballVelY += imu.accel.y * 0.4f;
    ballVelX *= 0.88f;
    ballVelY *= 0.88f;

    // Try X movement
    float testX = ballX + ballVelX;
    if (collidesAny(testX, ballY)) {
        ballVelX = 0;
    } else {
        ballX = testX;
    }

    // Try Y movement
    float testY = ballY + ballVelY;
    if (collidesAny(ballX, testY)) {
        ballVelY = 0;
    } else {
        ballY = testY;
    }

    // Draw ball (only if moved)
    int drawX = (int)ballX;
    int drawY = (int)ballY;
    if (drawX != prevDrawX || drawY != prevDrawY) {
        M5.Display.fillCircle(prevDrawX, prevDrawY, ballR + 1, BLACK);
        drawWalls();
        drawGoal();
        M5.Display.fillCircle(drawX, drawY, ballR, YELLOW);
        prevDrawX = drawX;
        prevDrawY = drawY;
    }

    // Win check
    if (abs(ballX - goalX) < (goalSize / 2 + ballR) &&
        abs(ballY - goalY) < (goalSize / 2 + ballR)) {
        mazeWon = true;
        M5.Display.fillScreen(BLACK);
        M5.Display.setTextColor(GREEN, BLACK);
        M5.Display.setTextSize(3);
        M5.Display.setCursor(4, 60);
        M5.Display.print("You");
        M5.Display.setCursor(4, 95);
        M5.Display.print("Win!");
        M5.Display.setTextSize(1);
        M5.Display.setTextColor(DARKGREY, BLACK);
        M5.Display.setCursor(8, 160);
        M5.Display.print("A: play again");
        M5.Display.setCursor(8, 175);
        M5.Display.print("B: menu");
        // Victory melody
        M5.Speaker.tone(523, 150);
        delay(160);
        M5.Speaker.tone(659, 150);
        delay(160);
        M5.Speaker.tone(784, 300);
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
