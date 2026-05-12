#include "common.h"
#include "menu.h"
#include "reaction.h"
#include "magic8.h"
#include "maze.h"

AppState currentState = STATE_MENU;

void switchState(AppState newState) {
    currentState = newState;
    switch (newState) {
        case STATE_MENU:      enterMenu();     break;
        case STATE_REACTION:  enterReaction();  break;
        case STATE_MAGIC8BALL: enterMagic8();   break;
        case STATE_TILTMAZE:  enterMaze();      break;
    }
}

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
