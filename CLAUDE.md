# m5-stickc

M5StickC Plus 2 project using PlatformIO + M5Unified on ESP32.

## Git Rules
- Do NOT add `Co-Authored-By` lines to commits

## Critical Rules

### Use M5Unified directly — NOT the StickCP2 wrapper
The `M5StickCPlus2` library's `StickCP2` global object has reference members that are vulnerable
to C++ static initialization order issues. As binaries grow, the references can bind to null,
causing random crashes. **Always use `M5.` instead of `StickCP2.`**

### IMU requires explicit update
Call `M5.Imu.update()` before `M5.Imu.getImuData()` — otherwise values are stale/frozen.

### Speaker volume
Default buzzer volume is near-silent. Set `M5.Speaker.setVolume(255)` before `tone()`.

## Build & Upload

```bash
pio run              # build
pio run -t upload    # build + flash to device
pio device monitor   # serial monitor (115200 baud)
```

## Debugging Crashes

If the device is flashing/rebooting, capture the crash trace:
1. Connect serial at 115200 baud
2. Look for `Guru Meditation Error` in the output
3. Decode backtrace with: `xtensa-esp32-elf-addr2line -pfiaC -e .pio/build/m5stick-c/firmware.elf <address>`

## Tilt Maze Architecture
- 6 levels with static walls, moving walls (levels 4-5), and rotating arcs (level 6)
- Moving walls: `movingWalls[]` array updated each frame, checked in `collidesAny()`
- Rotating arcs: collision checks angle-based gaps on circles, rendered via `M5Canvas` sprite buffer for flicker-free animation
- **Wall corner rule**: H and V walls must NEVER share an endpoint — leave 15px gap for ball to navigate corners
- **Gap sizing**: gaps should be 4-6× ball diameter (ball radius = 4px, diameter = 8px)
- Level data: `const Wall[]` arrays + `const Level levels[]` table. Max 10 internal walls per level (14 total with borders)
- `allWalls[14]` holds borders + level walls. Moving walls and arcs are separate systems

## platformio.ini Notes
- `lib_ignore = DFRobot_GP8XXX` — required to fix build error in dependency
- Board `m5stick-c` works for the Plus 2 (M5Unified auto-detects the actual hardware)
