# M5 StickC Plus2 Mini Games

A collection of mini games for the [M5StickC Plus 2](https://shop.m5stack.com/products/m5stickc-plus2-esp32-mini-iot-development-kit), built with [PlatformIO](https://platformio.org/) and [M5Unified](https://github.com/m5stack/M5Unified).

## About

This project is a starting point for anyone diving into modular embedded development with M5Stack devices. The M5StickC Plus 2 packs a color display, IMU, speaker, buttons, and battery into a tiny package — making it a great platform to learn on. I built these games as a way to explore what the hardware can do: reading sensors, driving a display, handling input, and managing application state.

I'm by no means a professional embedded developer — this is a learning project and I'm experimenting as I go. If you see something that could be done better, I'd love to hear about it. Take any patterns or practices here as one learner's approach, not gospel.

## Games

### Reaction Timer

Test your reflexes! The screen fills red with "Wait...", then after a random 2–5 second delay it flashes green with "GO!". Press BtnA as fast as you can. Your reaction time is shown in milliseconds along with a rating:

- **Lightning!** — under 200 ms
- **Fast!** — under 350 ms
- **Good!** — under 500 ms
- **Try again!** — 500 ms and up

Press too early during the red screen and it catches you. Great for competing with friends and family to see who has the fastest reflexes.

### Magic 8-Ball

Ask a question, then shake the device. The screen shows a classic 8-ball graphic with "Shake me!" — give it a shake and after a "Thinking..." animation with animated dots, one of 20 possible answers appears inside the ball. Shake again for a new answer. Uses the built-in accelerometer to detect shakes (triggers above 2G of force).

### Tilt Maze

Navigate a yellow ball through an S-shaped maze by physically tilting the device. The ball moves with simulated physics — it accelerates with tilt and decelerates with friction, so it feels like rolling a real ball. Walls block your path and the ball slides along them naturally. Reach the green goal square to win and hear a victory melody. The maze uses the IMU (accelerometer) for tilt input and per-axis collision detection for smooth wall interactions.

## Hardware

- [M5StickC Plus 2](https://shop.m5stack.com/products/m5stickc-plus2-esp32-mini-iot-development-kit) — ESP32-based with 135x240 color display, 6-axis IMU, buzzer, two buttons, and battery

You can pick one up from the [M5Stack Official Store](https://shop.m5stack.com/). They make a whole range of modular devices worth exploring — from the [M5Stack Core](https://shop.m5stack.com/collections/m5-controllers) series to various [sensor units](https://shop.m5stack.com/collections/m5-sensor) and [hats](https://shop.m5stack.com/collections/m5-hat).

## Controls

| Input | Menu | In Game |
|-------|------|---------|
| **BtnA** (front) | Scroll through games | Game action (press / play again) |
| **BtnB** (side) | Select game | Back to menu |
| **Tilt** | — | Move ball (Tilt Maze) |
| **Shake** | — | Get answer (Magic 8-Ball) |

## Build & Flash

### Prerequisites

1. Install [PlatformIO](https://platformio.org/) (CLI or IDE plugin)
2. Connect your M5StickC Plus 2 via USB-C

### Commands

```bash
pio run              # build
pio run -t upload    # build + flash to device
pio device monitor   # serial monitor (115200 baud)
```

### Project Structure

```
m5-stickc/
├── src/
│   └── main.cpp         # all game code lives here
├── platformio.ini        # build configuration
└── README.md
```

## How It Works

The app is built as a simple state machine. Each game has an `enter` function (draws the initial screen) and an `update` function (handles input and display updates each frame). BtnB always returns to the menu from any game. The main loop runs at ~100 Hz and dispatches to whichever game is currently active.

## Contributing

Contributions, ideas, and feedback are welcome! Whether it's a new game, a bug fix, or a suggestion for better practices — feel free to open an issue or submit a pull request.

Some ideas for additions:
- More games (Simon Says, Snake, Pong)
- High score tracking with persistent storage
- Sound effects and music
- Difficulty levels

## License

This project is licensed under the [MIT License](LICENSE). Feel free to use, modify, and share.

## Author

Robert ([@bobbydotdesign](https://github.com/bobbydotdesign))
