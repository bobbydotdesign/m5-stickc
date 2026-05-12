#include "reaction.h"

static enum { RT_WAITING, RT_GO, RT_RESULT, RT_TOO_EARLY } rtPhase;
static unsigned long rtGoTime;
static unsigned long rtDelay;
static unsigned long rtWaitStart;
static unsigned long rtReactionMs;
static unsigned long rtBestMs = 0;

static void startReactionRound() {
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
