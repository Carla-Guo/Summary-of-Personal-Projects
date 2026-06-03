# Smart Stair Light with Dual mmWave Radar

[Chinese Version](README_CN.md)

This is a dual-radar smart stair lighting prototype based on **Seeed Studio XIAO ESP32-C6 + LD2410C mmWave radar + ESP-NOW**.

The goal of this project is to build a standalone stair lighting system that does not depend on Home Assistant, a Wi-Fi router, or any cloud service. Two mmWave radar nodes are placed at the top and bottom of the stairs. The system uses ESP-NOW to determine which side a person enters from, then plays a directional flowing light animation.

For the full development story and project background, please see the Hackster article:

https://www.hackster.io/carla-guo/building-a-smart-stair-light-with-esp32-c6-and-mmwave-radar-99085f

## Features

- Dual mmWave radar direction detection
- ESP-NOW peer-to-peer wireless communication
- No Wi-Fi router required
- No Home Assistant required
- Upstairs and downstairs direction detection
- Entrance preview lighting, flowing animation, hold-on state, and delayed turn-off
- Finite state machine architecture for radar handling, passage detection, and LED animation


## System Architecture

This project uses two ESP32-C6 nodes:

| Node | Location | Role | Firmware |
| --- | --- | --- | --- |
| Top node | Top of the stairs | Reads the top radar and sends radar data through ESP-NOW | `xiao_mmwave_espnow_top.ino` |
| Bottom node | Bottom of the stairs | Reads the bottom radar, receives top radar data, and controls the LED strip | `xiao_mmwave_led_espnow_bottom_v3.ino` |

The data flow is:

```text
Top radar -> Top ESP32-C6 -> ESP-NOW -> Bottom ESP32-C6 -> LED strip
Bottom radar --------------------------^
```

The bottom node is the main controller. It handles the local bottom radar, the remote top radar data, and the LED strip.

## Hardware

Recommended hardware:

- 2 x Seeed Studio XIAO ESP32-C6
- 2 x LD2410C or compatible mmWave radar modules
- 1 x WS2812/NeoPixel-compatible LED strip
- 12V power supply
- 5V buck converter or the LED Driver Board from this project
- Jumper wires, or the custom PCB prototype included in this project

> Note: Do not power the LED strip directly from the ESP32-C6. Use a separate power supply or LED Driver Board for the LED strip, and make sure all grounds are connected together.

## Software

You can use either Arduino IDE or PlatformIO.

Required libraries:

- ESP32 Arduino Core, preferably a 3.x version with ESP32-C6 support
- `mmwave_for_xiao`
- `Adafruit NeoPixel`

In Arduino IDE, select the correct ESP32-C6 board. If you are using Seeed XIAO ESP32-C6, select the XIAO ESP32-C6 board or a compatible configuration.

## Quick Start

Before the first upload, you need to read the **MAC address of the bottom ESP32-C6 node**, then paste that MAC address into the top node firmware. The top node needs this address so it knows where to send ESP-NOW packets.

Recommended upload order:

1. Use `ESP_MAC.ino` to read the MAC address of the bottom node
2. Replace the MAC address in `xiao_mmwave_espnow_top.ino`
3. Upload the top firmware to the top ESP32-C6 node
4. Upload the bottom firmware to the bottom ESP32-C6 node

### 1. Read the Bottom Node MAC Address

Connect the **bottom ESP32-C6 node** to your computer.

Open and upload:

```text
smart_stair_light_with_mmWave_radar/ESP_MAC.ino
```

Open the serial monitor and set the baud rate to:

```text
115200
```

You should see output similar to:

```text
STA MAC: E4:B3:23:B5:17:3C
```

Save this MAC address. This is the MAC address of the bottom node.

### 2. Replace the MAC Address in the Top Firmware

Open:

```text
smart_stair_light_with_mmWave_radar/xiao_mmwave_espnow_top.ino
```

Find this line:

```cpp
uint8_t peerAddress[] = {0xE4, 0xB3, 0x23, 0xB5, 0x17, 0x3C};
```

Replace it with the MAC address you read from the bottom node.

For example, if the serial monitor shows:

```text
STA MAC: AA:BB:CC:11:22:33
```

Then the code should become:

```cpp
uint8_t peerAddress[] = {0xAA, 0xBB, 0xCC, 0x11, 0x22, 0x33};
```

### 3. Upload the Top Firmware

Connect the **top ESP32-C6 node** to your computer.

Upload:

```text
smart_stair_light_with_mmWave_radar/xiao_mmwave_espnow_top.ino
```

After uploading, the top node will read the upper mmWave radar and continuously send radar data to the bottom node through ESP-NOW.

### 4. Upload the Bottom Firmware

Reconnect the **bottom ESP32-C6 node** to your computer.

Upload:

```text
smart_stair_light_with_mmWave_radar/xiao_mmwave_led_espnow_bottom_v3.ino
```

After uploading, the bottom node will:

- Read the bottom mmWave radar
- Receive radar data from the top node
- Determine the walking direction
- Control the LED strip animation

## Wiring Notes

The main pins used by the current firmware are listed below.

### Bottom Node

LED strip data pin:

```cpp
#define LED_PIN A0
```

LED count:

```cpp
#define NUMPIXELS 60
```

If your LED strip has a different number of pixels, update `NUMPIXELS`.

### Top Node

The top node uses UART to communicate with the mmWave radar:

```cpp
COMSerial.begin(9600, SERIAL_8N1, D7, D6);
```

If your radar TX/RX wiring is different, update these pins according to your hardware.

## Common Parameters

You can tune these parameters in `xiao_mmwave_led_espnow_bottom_v3.ino`:

| Parameter | Description |
| --- | --- |
| `NUMPIXELS` | Total number of LEDs |
| `LED_BRIGHTNESS` | Global LED brightness |
| `PREVIEW_LED` | Number of entrance preview LEDs |
| `LED_INTERVAL_MS` | Flow animation speed |
| `HOLD_ON_MS` | Hold time before turning off after no presence is detected |
| `WAIT_TIMEOUT_MS` | Maximum time to wait for the second radar confirmation |
| `REMOTE_TIMEOUT_MS` | Timeout for detecting top node disconnection |
| `DEBOUNCE_MS` | Radar presence debounce time |

## How It Works

The firmware is split into three finite state machines:

| Module | Responsibility |
| --- | --- |
| Radar FSM | Reads local and remote radar data and outputs stable presence states |
| Passage FSM | Determines upstairs/downstairs direction based on trigger order |
| LED FSM | Plays preview, flow, hold, and turn-off animations |

This keeps radar reading, direction detection, and LED animation independent from each other. The system is more stable than placing all logic directly inside `loop()`.

## Troubleshooting

If the system does not work as expected, check the following:

1. Make sure the bottom node MAC address is correctly written into the top firmware
2. Make sure both top and bottom nodes are ESP32-C6 boards and were uploaded successfully
3. Make sure both radar modules are powered correctly
4. Make sure radar TX/RX wires are not reversed
5. Make sure the LED strip has enough power and shares GND with the ESP32-C6
6. Make sure `NUMPIXELS` matches the real LED strip length
7. Open the serial monitor and check whether ESP-NOW sending succeeds

## File Overview

| File | Description |
| --- | --- |
| `ESP_MAC.ino` | Reads the STA MAC address of an ESP32-C6 |
| `xiao_mmwave_espnow_top.ino` | Top node firmware, reads top radar and sends data |
| `xiao_mmwave_led_espnow_bottom_v3.ino` | Bottom node firmware, handles direction detection and LED control |
| `hardwave/` | PCB or hardware-related files |

## Project Status

This is an actively evolving prototype. The current version has validated:

- Dual-radar presence detection
- ESP-NOW communication
- Upstairs/downstairs direction detection
- LED flowing animation
- Radar debounce handling
- Delayed turn-off after no presence is detected

Possible future improvements:

- Web configuration page
- OTA firmware updates
- Ambient-light-based brightness adjustment
- More detailed position tracking using radar distance gates
- Multi-stair or multi-node expansion
