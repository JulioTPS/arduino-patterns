# ESP32-H2 SH1122 Scrolling Display

An Arduino project for the **ESP32-H2** using an **SH1122 display**.  
The display scrolls infinitely, showing either a predefined pattern or a randomly generated one.

## Features

- Infinite scrolling animation
- Predefined pattern mode
- Random pattern mode
- Button controls:
  - Increase scroll speed
  - Decrease scroll speed
  - Toggle between predefined and random patterns

This project uses the **u8g2** library made by *olikraus*. Unfortunately, it is currently the only Arduino-compatible library that supports the SH1122 display.

### Limitations

- u8g2 does **not** allow changing the grayscale value of individual pixels; grayscale can only be set for the entire screen.
- The included **u8x8** API allows per-tile grayscale control, but it is limited to **8×8 tiles**. Some workarounds could be made for the font but not the color.
- The previous library, **u8glib**, supported grayscale on some displays, but it didn't support the SH1122 at all.

## Project Status / Future Work

Due to the grayscale limitations of u8g2, future development will focus making the project with the **esp-sh1122** library made by *myles-parfeniuk* that uses the ESP-IDF framework instead of Arduino.  
This should allow finer grayscale control and more flexibility for rendering.
