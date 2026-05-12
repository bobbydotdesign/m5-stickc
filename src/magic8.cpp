#include "magic8.h"

static const char* magic8Answers[] = {
    "It is certain", "Decidedly so", "Without a doubt",
    "Yes definitely", "Rely on it", "As I see it yes",
    "Most likely", "Outlook good", "Yes",
    "Signs say yes",
    "Hazy try again", "Ask again later",
    "Better not say", "Cannot predict now",
    "Focus ask again",
    "Don't count on it", "My reply is no",
    "Sources say no", "Outlook not good",
    "Very doubtful"
};
static const int magic8Count = 20;

static enum { M8_IDLE, M8_THINKING, M8_ANSWER } m8Phase;
static unsigned long m8ThinkStart;
static unsigned long m8AnswerTime;
static int m8DotCount;
static unsigned long m8LastDot;

static bool detectShake() {
    M5.Imu.update();
    auto imu = M5.Imu.getImuData();
    float mag = sqrtf(imu.accel.x * imu.accel.x +
                      imu.accel.y * imu.accel.y +
                      imu.accel.z * imu.accel.z);
    return mag > 2.0f;
}

static void drawMagic8Idle() {
    M5.Display.fillScreen(BLACK);
    M5.Display.fillTriangle(10, 30, 125, 30, 67, 170, TFT_NAVY);
    M5.Display.setTextColor(WHITE, TFT_NAVY);
    M5.Display.setTextSize(4);
    M5.Display.setCursor(55, 84);
    M5.Display.print("8");
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
            if (now - m8ThinkStart >= 1500) {
                int idx = random(0, magic8Count);
                m8Phase = M8_ANSWER;
                m8AnswerTime = now;
                M5.Display.fillScreen(BLACK);
                M5.Display.fillTriangle(10, 30, 125, 30, 67, 170, TFT_NAVY);
                M5.Display.setTextColor(WHITE);
                const char* answer = magic8Answers[idx];
                int len = strlen(answer);
                M5.Display.setTextSize(2);
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
                M5.Display.setTextColor(WHITE, BLACK);
                M5.Display.setTextSize(2);
                M5.Display.setCursor(14, 180);
                M5.Display.print("Shake me!");
                M5.Display.setCursor(4, 210);
                drawBackHint(WHITE);
                M5.Speaker.tone(523, 100);
                delay(110);
                M5.Speaker.tone(659, 100);
                delay(110);
                M5.Speaker.tone(784, 200);
            }
            break;
        }

        case M8_ANSWER:
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
