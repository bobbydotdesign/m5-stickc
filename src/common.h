#pragma once
#include <M5Unified.h>

// --- State machine ---
enum AppState {
    STATE_MENU,
    STATE_REACTION,
    STATE_MAGIC8BALL,
    STATE_TILTMAZE
};

extern AppState currentState;
void switchState(AppState newState);

// --- Arrow drawing helpers ---
inline void drawDownArrow(uint16_t color) {
    int x = M5.Display.getCursorX();
    int y = M5.Display.getCursorY();
    M5.Display.fillTriangle(x + 4, y + 2, x + 12, y + 2, x + 8, y + 12, color);
    M5.Display.setCursor(x + 16, y);
}

inline void drawRightArrow(uint16_t color) {
    int x = M5.Display.getCursorX();
    int y = M5.Display.getCursorY();
    M5.Display.fillTriangle(x + 2, y + 2, x + 2, y + 12, x + 12, y + 7, color);
    M5.Display.setCursor(x + 16, y);
}

inline void drawLeftArrow(uint16_t color) {
    int x = M5.Display.getCursorX();
    int y = M5.Display.getCursorY();
    M5.Display.fillTriangle(x + 12, y + 2, x + 12, y + 12, x + 2, y + 7, color);
    M5.Display.setCursor(x + 16, y);
}

inline void drawBackHint(uint16_t color) {
    M5.Display.print("B:");
    drawLeftArrow(color);
}

inline void drawRetryBackHint(uint16_t color) {
    M5.Display.setCursor(4, 210);
    M5.Display.print("A:Again B:");
    drawLeftArrow(color);
}
