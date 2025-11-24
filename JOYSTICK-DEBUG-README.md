# Joystick Debug Testing Guide

## Problem Summary
Joystick appears connected and zoom in/out works, but other functionality (panning, creating boxes, cycling) doesn't work as expected. Need to debug the event mapping and axis reading.

## What Was Done

### 1. Added Debug Logging
- Modified `src/joystick.c` to log all events when built with DEBUG flag
- Logs to `/tmp/joystick_debug.log`
- Captures:
  - Device open/close events
  - All axis events (EV_ABS) with codes and values
  - All button events (EV_KEY) with codes and mappings
  - Sync events (EV_SYN)
  - Unknown events

### 2. Created Test Canvas
- `joystick-test-canvas.txt` - Simple canvas with instructions for each mode
- Designed specifically for testing joystick controls
- Clear visual reference for what each button/axis should do

### 3. Created Test Scripts

#### test-joystick.sh
Run this in a **SEPARATE TERMINAL** (not in Claude Code session):
```bash
./test-joystick.sh
```
- Loads the test canvas
- Enables debug logging
- Provides testing instructions
- Shows log summary after exit

#### analyze-joystick-log.sh
Run this **AFTER** testing to analyze the captured events:
```bash
./analyze-joystick-log.sh
```
- Shows statistics on event types
- Lists detected axis codes
- Lists detected button codes
- Shows axis value ranges
- Identifies unknown/unmapped events

## Testing Workflow

### Step 1: Run Test (in separate terminal)
```bash
cd /home/be-dev-agent/projects/boxes-live
./test-joystick.sh
```

**Test these operations:**
1. **Navigation Mode (default)**:
   - Move left stick → should pan viewport
   - Press A button → should zoom in
   - Press B button → should zoom out
   - Press X button → should cycle to next box
   - Press Y button → should create new box

2. **Edit Mode** (after selecting a box):
   - Move left stick → should move the selected box
   - Press X button → should cycle box color
   - Press A button → should enter parameter mode
   - Press B button → should return to navigation mode

3. **Parameter Mode** (from edit mode):
   - Move stick up/down → should select parameter (width/height/color)
   - Move stick left/right → should adjust parameter value
   - Press A button → confirm and return to edit mode

**Take notes on what works and what doesn't!**

### Step 2: Analyze Debug Log
```bash
./analyze-joystick-log.sh
```

Look for:
- **Axis codes**: Should be 0 (ABS_X) and 1 (ABS_Y) for left stick
- **Button codes**: Should map to expected values
- **Unknown events**: Events we're not handling correctly
- **Value ranges**: Axis values should be around -32768 to +32767

### Step 3: Compare Expected vs Actual

#### Expected Axis Mapping:
- Left stick X = ABS_X (code 0)
- Left stick Y = ABS_Y (code 1)

#### Expected Button Mapping:
- Button A = BTN_GAMEPAD + 0 (code 0x130 = 304)
- Button B = BTN_GAMEPAD + 1 (code 0x131 = 305)
- Button X = BTN_GAMEPAD + 2 (code 0x132 = 306)
- Button Y = BTN_GAMEPAD + 3 (code 0x133 = 307)

Or for some controllers:
- Buttons start at BTN_JOYSTICK (0x120 = 288)

## Common Issues to Check

### Issue: Panning doesn't work
**Check in log:**
- Are EV_ABS events with code 0 and 1 being received?
- Are the axis values actually changing when you move the stick?
- Is the deadzone (0.15) too high?
- Are the values being normalized correctly?

**Look for:**
```
EV_ABS: code=0 value=<some number>
  -> AXIS_X = <number> (normalized: <-1.0 to 1.0>)
```

### Issue: Buttons don't work (except zoom)
**Check in log:**
- What button codes are being received?
- Are they being mapped to button[0-15] correctly?
- Are the button mappings (BUTTON_A=0, BUTTON_B=1, etc.) correct?

**Look for:**
```
EV_KEY (BTN_GAMEPAD): code=304 (0x130) value=1 -> button=0
```

### Issue: Wrong axis mapped
**Check in log:**
- Your controller might use different axis codes
- ABS_X might not be code 0, could be ABS_HAT0X (16) or similar

## Next Steps After Testing

1. **Share the debug log** with me:
   ```bash
   cat /tmp/joystick_debug.log
   ```

2. **Report what worked and what didn't**:
   - Which buttons/axes responded correctly?
   - Which operations failed?
   - Any unexpected behavior?

3. **I'll analyze the log to identify**:
   - Incorrect axis code mappings
   - Wrong button code mappings
   - Deadzone issues
   - Value range problems

4. **We'll fix the issues**:
   - Update axis code constants if needed
   - Fix button mapping logic
   - Adjust deadzone or scaling
   - Update input_unified.c if event processing has issues

## Current Code Locations

- **Joystick driver**: `src/joystick.c`, `include/joystick.h`
- **Input processing**: `src/input_unified.c`, `include/input_unified.h`
- **Event handling**: `src/input.c`
- **Main loop**: `src/main.c` (lines 266-274 handle joystick polling)

## Debugging Without Corrupting Session

**IMPORTANT**: Do NOT run `boxes-live` in the Claude Code terminal session. The ncurses interface will corrupt the terminal output. Always use:
- A separate terminal window/tab, OR
- The test script which handles cleanup

## References

- Linux Input Event Codes: https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h
- Evdev documentation: https://www.kernel.org/doc/html/latest/input/input.html
