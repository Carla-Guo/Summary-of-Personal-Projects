# XIAO Community Projects Weekly

*17 projects · April 17, 2026*

*Discover what makers are building with Seeed Studio XIAO development boards. This week: 17 new community projects spanning Smart Home, Robotics, Telecommunication, Tools & Accessories, LED Lighting, and Gaming.*

---

## larapaper by usetrmnl

A self-hosted, community-driven server built with **PHP Laravel** that brings Recipe support to the **Seeed Studio XIAO 7.5 inch ePaper Panel**. This project enables makers to display dynamic content like weather, calendars, and notifications on low-power ePaper screens without relying on cloud services. It is designed for privacy-conscious users who want full control over their smart home data.

**Hardware Recommendation**
- **XIAO:** [Seeed Studio XIAO ESP32-S3](https://www.seeedstudio.com/XIAO-ESP32S3-p-5627.html) (WiFi + PSRAM for image buffering)
- **Expansion:** [ePaper Driver Board V2](https://www.seeedstudio.com/ePaper-breakout-Board-for-XIAO-V2-p-6374.html)

<https://github.com/usetrmnl/larapaper>

---

## trackball by monroewilliams

A fully custom trackball built from scratch using **Seeeduino XIAO** or **Adafruit QT Py** paired with optical mouse sensors harvested from standard computer mice. The design features Z-axis ball rotation for scroll functionality, making it a unique input device for productivity or accessibility use cases. It demonstrates how everyday optical sensors can be repurposed for custom hardware projects.

**Hardware Recommendation**
- **XIAO:** [Seeed Studio XIAO SAMD21](https://www.seeedstudio.com/Seeeduino-XIAO-Arduino-Microcontroller-SAMD21-Cortex-M0+-p-4426.html) (compact form factor)
- **Expansion:** [Grove Shield for Seeeduino XIAO](https://www.seeedstudio.com/Grove-Shield-for-Seeeduino-XIAO-p-4621.html)

<https://github.com/monroewilliams/trackball>

---

## esp32-photoframe by aitjcize

Feature-rich **ESP32-based firmware** for e-paper displays that transforms any compatible panel into a smart digital photo frame. Supports **Seeed Studio XIAO EE02/EE04**, **Waveshare PhotoPainter**, and **reTerminal E1002**, enabling WiFi-connected image slideshows with minimal power consumption. The project is ideal for home decor, office signage, or low-power information kiosks.

**Hardware Recommendation**
- **XIAO:** [Seeed Studio XIAO ESP32-S3](https://www.seeedstudio.com/XIAO-ESP32S3-p-5627.html) (WiFi + PSRAM for image storage)
- **Expansion:** [ePaper Driver Board V2](https://www.seeedstudio.com/ePaper-breakout-Board-for-XIAO-V2-p-6374.html)

<https://github.com/aitjcize/esp32-photoframe>

---

## xiaomao by tigard-tools

An open-source development board designed specifically for **hardware hacking** and **implant-style projects**, originally based on **Seeed Studio XIAO SAMD21** and now redesigned around **RP2040** for more processing power. The board exposes debug interfaces and test points for reverse engineering, security research, and educational purposes. It fills a gap in the maker ecosystem for compact, hacker-friendly embedded tooling.

**Hardware Recommendation**
- **XIAO:** [Seeed Studio XIAO RP2040](https://www.seeedstudio.com/XIAO-RP2040-v1-0-p-5026.html) (dual-core, good for debugging workflows)
- **Expansion:** null

<https://github.com/tigard-tools/xiaomao>

---

## Modxo by Team-Resurgent

**RP2040 firmware** that transforms boards like **XIAO RP2040** into Original Xbox-compatible **LPC peripheral devices**, enabling users to load BIOS images and play classic Xbox games on original hardware. This project bridges modern microcontroller technology with retro gaming, providing an accessible way to preserve and play Xbox games without original hard drive equipment. It is a niche but dedicated tool for the retro gaming and Xbox modding community.

**Hardware Recommendation**
- **XIAO:** [Seeed Studio XIAO RP2040](https://www.seeedstudio.com/XIAO-RP2040-v1-0-p-5026.html)
- **Expansion:** null

<https://github.com/Team-Resurgent/Modxo>

---

## Hackflight by simondlevy

A minimalist **flight-control toolkit** for multirotor drones, built with header-only C++ classes for a highly composable and readable architecture. The project runs on **XIAO ESP32-C6** as well as **Teensy 4.0**, leveraging the ESP32-C6's WiFi 6 capability for wireless configuration and telemetry. It is aimed at makers who want a lightweight, hackable alternative to commercial flight controller firmware.

**Hardware Recommendation**
- **XIAO:** [Seeed Studio XIAO ESP32-C6](https://www.seeedstudio.com/Seeed-Studio-XIAO-ESP32C6-p-5884.html) (WiFi 6 for telemetry)
- **Expansion:** [Grove Shield for Seeeduino XIAO](https://www.seeedstudio.com/Grove-Shield-for-Seeeduino-XIAO-p-4621.html)

<https://github.com/simondlevy/Hackflight>

---

## espradio by tinygo-org

A **TinyGo package** providing wireless communication capabilities for Espressif **ESP32xx microcontrollers**, including **WiFi**, **DHCP**, **DNS**, **NTP**, and **MQTT** protocols. The library has been tested on **XIAO ESP32-C3** and **XIAO ESP32-S3**, making it a solid foundation for IoT sensor nodes and remote monitoring devices. It brings the simplicity of Go to embedded wireless projects without the overhead of a full RTOS.

**Hardware Recommendation**
- **XIAO:** [Seeed Studio XIAO ESP32-C3](https://www.seeedstudio.com/Seeed-XIAO-ESP32C3-p-5431.html) (compact, low-power)
- **Expansion:** [Grove Shield for Seeeduino XIAO](https://www.seeedstudio.com/Grove-Shield-for-Seeeduino-XIAO-p-4621.html)

<https://github.com/tinygo-org/espradio>

---

## micropython-reticulum by varna9000

A pure **Python** implementation of the **Reticulum** encrypted mesh networking stack running on **ESP32 microcontrollers**, tested on **Seeed XIAO ESP32-S3**. Reticulum is designed for building resilient, long-range, low-power networks in remote or infrastructure-free environments. This project enables makers to create off-grid communication nodes, mesh networks, and resilient IoT systems using familiar MicroPython syntax.

**Hardware Recommendation**
- **XIAO:** [Seeed Studio XIAO ESP32-S3](https://www.seeedstudio.com/XIAO-ESP32S3-p-5627.html) (MicroPython + WiFi support)
- **Expansion:** [Grove Shield for Seeeduino XIAO](https://www.seeedstudio.com/Grove-Shield-for-Seeeduino-XIAO-p-4621.html)

<https://github.com/varna9000/micropython-reticulum>

---

## pico-cec by gkoh

An **HDMI CEC to USB HID keyboard adapter** that lets you control a PC or media center using your TV remote, built on **XIAO RP2350** or Raspberry Pi Pico. CEC (Consumer Electronics Control) allows your TV's remote to send commands over the HDMI cable, which this adapter translates into keyboard inputs. It is a clean, cable-minimal solution for home theater PC setups without needing a dedicated remote receiver.

**Hardware Recommendation**
- **XIAO:** [Seeed Studio XIAO RP2350](https://www.seeedstudio.com/Seeed-XIAO-RP2350-p-5944.html) (dual-core M33 + RISC-V)
- **Expansion:** null

<https://github.com/gkoh/pico-cec>

---

## plainFlightController by plainFlight

**Flight stabilisation software** for RC multirotor aircraft that runs on **XIAO ESP32-S3** paired with an **MPU6050 IMU** sensor. The software handles real-time sensor fusion and PID tuning to keep the aircraft stable during flight, with configuration accessible via a serial terminal or web interface. It targets hobbyists who want an open, customizable alternative to closed-source flight controllers.

**Hardware Recommendation**
- **XIAO:** [Seeed Studio XIAO ESP32-S3](https://www.seeedstudio.com/XIAO-ESP32S3-p-5627.html) (dual-core for real-time processing)
- **Expansion:** [Grove Shield for Seeeduino XIAO](https://www.seeedstudio.com/Grove-Shield-for-Seeeduino-XIAO-p-4621.html)

<https://github.com/plainFlight/plainFlightController>

---

## ShellOS by passionateSandy2004

A lightweight **custom operating system** for **ESP32** and **XIAO ESP32-C6** boards that brings shell access, WiFi configuration, and a basic filesystem to embedded projects. It abstracts away the complexity of ESP-IDF while providing a familiar command-line interface for managing network connections and files on the device. This project is perfect for makers who want a Unix-like experience directly on their XIAO hardware.

**Hardware Recommendation**
- **XIAO:** [Seeed Studio XIAO ESP32-C6](https://www.seeedstudio.com/Seeed-Studio-XIAO-ESP32C6-p-5884.html) (WiFi 6 + dual-core RISC-V)
- **Expansion:** [Seeeduino XIAO Expansion Board](https://www.seeedstudio.com/Seeeduino-XIAO-Expansion-board-p-4746.html)

<https://github.com/passionateSandy2004/ShellOS---OS-made-Just-For-ESP32-Boards>

---

## Smart Matter Button using XIAO ESP32 C6 by techiesms

A beginner-friendly video tutorial demonstrating how to build a smart **Matter** button using **Seeed XIAO ESP32 C6** with **Arduino** framework. Matter is the new smart home standard backed by Apple, Google, and Amazon, enabling seamless cross-platform device control. This project is an excellent entry point for makers looking to create custom smart home hardware without dealing with complex certification processes.

![Smart Matter Button](https://i.ytimg.com/vi/wokw847hjKM/maxresdefault.jpg)

**Hardware Recommendation**
- **XIAO:** [Seeed Studio XIAO ESP32-C6](https://www.seeedstudio.com/Seeed-Studio-XIAO-ESP32C6-p-5884.html) (Thread + WiFi 6 for Matter)
- **Expansion:** [Seeeduino XIAO Expansion Board](https://www.seeedstudio.com/Seeeduino-XIAO-Expansion-board-p-4746.html)

<https://www.youtube.com/watch?v=wokw847hjKM>

---

## DIY ESPHome Controller with Seeed Xiao ESP32-C3 by Licky's Smart Home Channel

A hands-on guide to building a custom **ESPHome** smart home device using **Seeed Xiao ESP32-C3**, integrated seamlessly into **Home Assistant** for monitoring and automation. ESPHome is a popular framework that lets you define device behaviour in YAML and flashed over-the-air, making smart home projects highly maintainable. The project covers wiring, configuration, and sensor integration for a complete self-built smart home node.

![DIY ESPHome Controller](https://i.ytimg.com/vi/_xxoEm9-eiI/maxresdefault.jpg)

**Hardware Recommendation**
- **XIAO:** [Seeed Studio XIAO ESP32-C3](https://www.seeedstudio.com/Seeed-XIAO-ESP32C3-p-5431.html) (WiFi + BLE for sensor integration)
- **Expansion:** [Grove Shield for Seeeduino XIAO](https://www.seeedstudio.com/Grove-Shield-for-Seeeduino-XIAO-p-4621.html)

<https://www.youtube.com/watch?v=_xxoEm9-eiI>

---

## VFD Display with XIAO ESP32S3 NTP Clock by iHayri1

A visually striking **VFD (Vacuum Fluorescent Display) clock** with animated effects, synchronized to accurate time via **NTP** on **Seeed XIAO ESP32S3**. VFD displays offer a distinctive retro-futuristic glow compared to LCD or OLED, making this project a standout desk accessory. The clock fetches time automatically over WiFi and can be extended to display weather, calendar events, or custom messages.

![VFD Display NTP Clock](https://i.ytimg.com/vi/ToGKmtNXNh8/maxresdefault.jpg)

**Hardware Recommendation**
- **XIAO:** [Seeed Studio XIAO ESP32-S3](https://www.seeedstudio.com/XIAO-ESP32S3-p-5627.html) (WiFi for NTP sync + PSRAM for display buffers)
- **Expansion:** null

<https://www.youtube.com/watch?v=ToGKmtNXNh8>

---

## Solar Powered Water Tank Level Sensor V2 by Electronic Clinic

A completely **solar powered wireless water tank level sensor** built with **Xiao ESP32C3** using **BLE** to transmit readings to a smartphone or home automation system. Version 2 improves power efficiency and sensor accuracy over the original, making it suitable for remote cabins, farms, or off-grid installations where power is scarce. The project demonstrates how to combine energy harvesting, low-power BLE, and durable sensor packaging for long-term outdoor deployment.

![Solar Water Tank Level Sensor](https://i.ytimg.com/vi/J7URXH6Qo6Q/maxresdefault.jpg)

**Hardware Recommendation**
- **XIAO:** [Seeed Studio XIAO ESP32-C3](https://www.seeedstudio.com/Seeed-XIAO-ESP32C3-p-5431.html) (ultra-low power @ 44μA in deep sleep)
- **Expansion:** [Grove Shield for Seeeduino XIAO](https://www.seeedstudio.com/Grove-Shield-for-Seeeduino-XIAO-p-4621.html)

<https://www.youtube.com/watch?v=J7URXH6Qo6Q>

---

## Voice-Controlled Robot Dog by Tech Talkies

A **voice-controlled quadruped robot dog** powered by **Seeed XIAO ESP32S3**, responding to spoken commands to walk, turn, and perform predefined movements. The project combines motor control, real-time voice processing, and inverse kinematics to produce lifelike motion on a compact four-legged platform. It is an engaging way to explore legged robotics, speech recognition, and embedded AI on an affordable, well-documented hardware stack.

![Voice-Controlled Robot Dog](https://i.ytimg.com/vi/QQHyo7KlaQk/maxresdefault.jpg)

**Hardware Recommendation**
- **XIAO:** [Seeed Studio XIAO ESP32-S3 Sense](https://www.seeedstudio.com/XIAO-ESP32S3-Sense-p-5639.html) (onboard microphone for voice input)
- **Expansion:** [Bus Servo Driver Board](https://www.seeedstudio.com/Bus-Servo-Driver-Board-for-XIAO-p-6413.html) + [XIAO Bus Servo Adapter](https://www.seeedstudio.com/XIAO-Bus-Servo-Adapter-for-XIAO-p-6397.html)

<https://www.youtube.com/watch?v=QQHyo7KlaQk>

---

## Embassy Rust + XIAO BLE - Digital Input by Sarmad Gulzar

A practical tutorial demonstrating **digital input handling** using the **Embassy Rust** async framework with **Seeed XIAO BLE** (nRF52840). Embassy is a Rust async executor designed for embedded systems, providing memory-safe concurrency without the overhead of an RTOS. This project is an excellent starting point for developers looking to write robust, event-driven embedded software in Rust on XIAO hardware.

![Embassy Rust + XIAO BLE](https://i.ytimg.com/vi/Rmd7PqXNT28/maxresdefault.jpg)

**Hardware Recommendation**
- **XIAO:** [Seeed Studio XIAO nRF52840](https://www.seeedstudio.com/Seeed-XIAO-BLE-nRF52840-p-5201.html) (BLE 5.0 + Cortex-M4)
- **Expansion:** [Grove Shield for Seeeduino XIAO](https://www.seeedstudio.com/Grove-Shield-for-Seeeduino-XIAO-p-4621.html)

<https://www.youtube.com/watch?v=Rmd7PqXNT28>

---

## XIAO Board Selection Guide

Use this reference table to choose the right XIAO board for your next project.

| Parameter | [SAMD21](https://www.seeedstudio.com/Seeeduino-XIAO-Arduino-Microcontroller-SAMD21-Cortex-M0+-p-4426.html) | [RP2040](https://www.seeedstudio.com/XIAO-RP2040-v1-0-p-5026.html) | [RP2350](https://www.seeedstudio.com/Seeed-XIAO-RP2350-p-5944.html) | [nRF52840](https://www.seeedstudio.com/Seeed-XIAO-BLE-nRF52840-p-5201.html) | [nRF52840 Sense](https://www.seeedstudio.com/Seeed-XIAO-BLE-Sense-nRF52840-p-5253.html) | [ESP32C3](https://www.seeedstudio.com/Seeed-XIAO-ESP32C3-p-5431.html) | [ESP32C5](https://www.seeedstudio.com/Seeed-Studio-XIAO-ESP32C5-p-6609.html) | [ESP32C6](https://www.seeedstudio.com/Seeed-Studio-XIAO-ESP32C6-p-5884.html) | [ESP32S3](https://www.seeedstudio.com/XIAO-ESP32S3-p-5627.html) | [ESP32S3 Sense](https://www.seeedstudio.com/XIAO-ESP32S3-Sense-p-5639.html) | [RA4M1](https://www.seeedstudio.com/Seeed-XIAO-RA4M1-p-5943.html) | [MG24](https://www.seeedstudio.com/Seeed-Studio-XIAO-MG24-p-6247.html) | [MG24 Sense](https://www.seeedstudio.com/Seeed-XIAO-MG24-Sense-p-6248.html) | [nRF54L15](https://www.seeedstudio.com/XIAO-nRF54L15-p-6493.html) | [nRF54L15 Sense](https://www.seeedstudio.com/XIAO-nRF54L15-Sense-p-6494.html) |
|-----------|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| **Chip** | SAMD21 | RP2040 | RP2350 | nRF52840 | nRF52840 | ESP32C3 | ESP32-C5 | ESP32C6 | ESP32S3 | ESP32S3 | RA4M1 | EFR32MG24 | EFR32MG24 | nRF54L15 | nRF54L15 |
| **Arch / Clock** | Cortex-M0+ @ 48MHz | Dual Cortex-M0+ @ 133MHz | Dual Cortex-M33 + RISC-V @ 150MHz | Cortex-M4 @ 64MHz | Cortex-M4 @ 64MHz | RISC-V @ 160MHz | RISC-V @ 240MHz | Dual RISC-V 160+20MHz | Dual Xtensa LX7 @ 240MHz | Dual Xtensa LX7 @ 240MHz | Cortex-M4 + FPU @ 48MHz | Cortex-M33 @ 78MHz | Cortex-M33 @ 78MHz | Dual-core M33+RISC-V @ 128MHz | Dual-core M33+RISC-V @ 128MHz |
| **SRAM** | 32 KB | 264 KB | 520 KB | 256 KB | 256 KB | 400 KB | 384 KB | 512 KB | 512 KB | 512 KB | 32 KB | 256 KB | 256 KB | 256 KB | 256 KB |
| **Internal Flash** | 256 KB | — | — | 1 MB | 1 MB | 4 MB | — | 4 MB | 384 KB ROM | 384 KB ROM | 256 KB | 1536 KB + 4 MB | 1536 KB + 4 MB | 1.5 MB | 1.5 MB |
| **Onboard Flash** | — | 2 MB | — | 2 MB | 2 MB | — | 8 MB | — | 8 MB | 8 MB | — | — | — | — | — |
| **Built-in Sensors** | — | — | — | — | IMU + Microphone | — | — | — | — | Camera + Microphone | — | — | IMU + Microphone | — | IMU + Microphone |
| **WiFi** | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ WiFi 6 (5G) | ✅ WiFi 6 | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **BLE** | ❌ | ❌ | ❌ | ✅ 5.0 | ✅ 5.0 | ✅ 5.0 | ✅ 5.0 | ✅ 5.0 | ✅ 5.0 | ✅ 5.0 | ❌ | ✅ 5.3 | ✅ 5.3 | ✅ 6.0 | ✅ 6.0 |
| **Low Power** | — | — | 50 μA | 5 μA | 5 μA | 44 μA | — | 15 μA | 14 μA | 26.5 mA | 45 μA | 1.95 μA | 1.95 μA | — | — |
| **Arduino** | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ |
| **PlatformIO** | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| **MicroPython** | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ | ✅ | ❌ | ✅ |
| **CircuitPython** | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **Zephyr** | ✅ | ✅ | ❌ | ✅ | ✅ | ✅ | ❌ | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ✅ | ✅ |
