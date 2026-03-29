# Relay Module 1

## Project Overview
This project is an **ESP32-based relay controller** designed to manage 16 relays via shift registers (74HC595). It uses the **ESP-NOW** protocol for wireless communication with a master controller (e.g., a screen or main unit) and supports **Over-the-Air (OTA)** updates using **ElegantOTA**.

### Main Technologies
- **Hardware**: ESP32 (NodeMCU-32S / DOIT DevKit V1), 74HC595 Shift Registers.
- **Framework**: Arduino (via PlatformIO).
- **Communication**: ESP-NOW (Low-latency, peer-to-peer).
- **OTA Updates**: ElegantOTA (v3.1.7).

---

## Building and Running

### Prerequisites
- **PlatformIO**: Install via VS Code extension or CLI (`pip install platformio`).

### Key Commands
- **Build**: `pio run`
- **Upload**: `pio run -t upload`
- **Monitor**: `pio run -t monitor`
- **Clean**: `pio run -t clean`

---

## Project Architecture

### Relay Control
The system manages 16 relay states in a boolean array (`registers[16]`). These states are shifted out to two daisy-chained **74HC595** shift registers using the following pins:
- `SER_Pin` (GPIO 14): Data Input (DS)
- `RCLK_Pin` (GPIO 12): Storage Register Clock (ST_CP)
- `SRCLK_Pin` (GPIO 13): Shift Register Clock (SH_CP)
- `outout_enablePin` (GPIO 5): Output Enable (OE)

### ESP-NOW Communication
- **Data Structure**: `struct_message` contains an integer `a` (Relay ID) and a boolean `b` (State).
- **Receive Callback (`OnDataRecv`)**: Updates the corresponding relay in the `registers` array and pushes the data to the shift registers.
- **Send Callback (`OnDataSent`)**: Logs transmission status; if a transmission fails, it resets the shift registers.
- **Peer Address**: `98:88:e0:04:e2:48` (Hardcoded as `broadcastAddress`).

### OTA Implementation
OTA is managed via `src/elegantota.cpp`. When a command with `myData.a == 255` is received via ESP-NOW, the system:
1. Sets `update = true`.
2. Starts a Wi-Fi Access Point (`SSID: Relay_module`, `Pass: 80100000`).
3. Starts a Web Server on port 80.
4. Enables the ElegantOTA web interface for firmware/filesystem uploads.

---

## Key Files
- `src/main.cpp`: Core logic, ESP-NOW handling, and shift register control.
- `src/elegantota.cpp`: Custom wrapper for ElegantOTA and Wi-Fi AP setup.
- `include/elegantota.h`: Function declarations for the OTA module.
- `platformio.ini`: Project configuration, library dependencies, and board settings.
- `src/partitions.csv`: Custom flash partition table (supports large OTA apps).

---

## Development Conventions
- **Pins**: Pin definitions are located in `src/main.cpp`.
- **States**: Relay states are active-high/active-low based on the shift register wiring (currently logic follows `registers` state).
- **Network**: The device operates in `WIFI_STA` mode for ESP-NOW but switches to `WIFI_AP` for OTA updates.
- **Error Handling**: Failed ESP-NOW transmissions trigger a relay reset for safety.
