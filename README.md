# M5StickC Plus 2 Mini Games

A collection of mini games for the [M5StickC Plus 2](https://shop.m5stack.com/products/m5stickc-plus2-esp32-mini-iot-development-kit), built with PlatformIO and M5Unified.

## Games

### Reaction Timer
Test your reflexes! The screen shows red with "Wait...", then flashes green after a random delay. Press the button as fast as you can and see your reaction time in milliseconds with a rating (Lightning / Fast / Good / Try again).

### Magic 8-Ball
Ask a question, then shake the device. After a brief "Thinking..." animation, the 8-ball reveals one of 20 possible answers. Shake again for another answer.

### Tilt Maze
Navigate a yellow ball through an S-shaped maze by tilting the device. Reach the green goal square to win and hear a victory melody.

## Hardware

- [M5StickC Plus 2](https://shop.m5stack.com/products/m5stickc-plus2-esp32-mini-iot-development-kit) (ESP32-based)

## Controls

| Button | Menu | In Game |
|--------|------|---------|
| **BtnA** (front) | Scroll through games | Game action (press/play again) |
| **BtnB** (side) | Select game | Back to menu |
| **Tilt** | — | Move ball (Tilt Maze) |
| **Shake** | — | Get answer (Magic 8-Ball) |

## Build & Flash

Requires [PlatformIO](https://platformio.org/).

```bash
pio run              # build
pio run -t upload    # build + flash to device
pio device monitor   # serial monitor (115200 baud)
```

## Author

Robert
