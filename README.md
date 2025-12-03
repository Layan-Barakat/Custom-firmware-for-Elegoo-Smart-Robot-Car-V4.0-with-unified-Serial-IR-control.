# Elegoo Smart Robot Car V4.0 – Custom Firmware

This repository contains a custom Arduino firmware for the Elegoo Smart Robot Car V4.0 (Arduino Uno + TB6612 motor driver shield).

The default Elegoo firmware is quite large and tightly coupled to the official mobile app.  
This project implements a **minimal, fully custom control layer** that focuses on:

- Direct Serial control from a PC (W/A/S/D/X, +/- for speed)
- Full support for the Elegoo IR remote (arrows, OK, 1/2/3)
- Continuous forward/backward driving
- Step-based turning (single press = small turn)
- Multiple speed levels (slow / medium / fast)

## Features

- [x] Motor control for both left and right wheels using the TB6612 driver
- [x] Serial command interface for quick testing and debugging
- [x] IR remote support using `IRremote` 2.6.0–style API
- [x] Configurable speed levels and fine-grained speed tuning
- [x] Clean, well-commented code focused on learning and extensibility

## Hardware

- Elegoo Smart Robot Car Kit V4.0
  - Arduino Uno compatible board
  - TB6612 motor driver shield
  - IR receiver module
  - DC motors and chassis

## How it works

The firmware directly drives the motor driver pins:

- Standby: `STBY = 3`
- Right motor (A): `PWMA = 5`, `AIN1 = 7`
- Left motor  (B): `PWMB = 6`, `BIN1 = 8`
- IR receiver: `RECV_PIN = 9`

The code exposes simple movement primitives:

- `startForwardContinuous()` / `startBackwardContinuous()`
- `stepTurnLeft()` / `stepTurnRight()`
- `stopMotors()`

On top of that, there are two control layers:

1. **Serial control (PC)**  
   - `W` / `S` – forward / backward (continuous)  
   - `A` / `D` – left / right (step turn)  
   - `X` – stop  
   - `1 / 2 / 3` – set speed level  
   - `+ / -` – fine speed tuning

2. **IR remote control (Elegoo remote)**  
   - Up / Down – forward / backward (continuous)  
   - Left / Right – step turn  
   - OK – stop  
   - 1 / 2 / 3 – set speed level

## Usage

1. Open `elegoo_car_custom_firmware.ino` in Arduino IDE.
2. Install `IRremote` (version 2.x – the API used in this code).
3. Select `Arduino Uno` as the board and the correct COM port.
4. If needed on your kit, temporarily remove the motor shield to upload.
5. Upload the sketch.
6. Power the car via battery pack and control it via:
   - USB Serial (Serial Monitor / Python script / etc.)
   - The included IR remote.

## Goals

This project was built to:

- Understand the motor driver and pin mappings without relying on the full Elegoo example.
- Create a clean, minimal codebase suitable for future extensions:
  - Bluetooth / Wi-Fi control
  - Autonomous modes
  - Integration with external ML / CV systems via a custom app.

## Credits

- Hardware and original example code: Elegoo Smart Robot Car V4.0.
- Custom firmware design, refactor, and control logic: Layan.
