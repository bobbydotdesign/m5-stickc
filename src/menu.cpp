#include "menu.h"

static const char* menuItems[] = { "Reaction", "8-Ball", "Tilt Maze" };
static const int menuCount = 3;
static int menuSelection = 0;

static void drawMenuItems() {
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
