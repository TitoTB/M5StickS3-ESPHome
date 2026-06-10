# Habbit Pocket

![Habbit Pocket](images/Habbit%20Pocket.png)

## M5Stick + ESPHome

**Habbit Pocket** turns the M5StickS3 into a small, customizable pocket controller for Home Assistant.

It uses ESPHome plus a few custom components to provide a compact interface for controlling any Home Assistant device, either manually from an on-screen device menu or by voice through the native Home Assistant Assist pipeline.

## What It Does

Habbit Pocket gives you two main ways to interact with your smart home:

- **Manual control** through a simple on-device menu.
- **Voice control** through the Home Assistant voice assistant.

The goal is to keep the device fast, minimal, and pocket-friendly: wake it only when you need it, choose a device, toggle it, or speak a command.

## How It Works

- Double-click the center button to wake the screen.
- Long-press the center button to start the voice assistant.
- Press the side button to open the device menu.
- In the device menu, press the side button to move between devices.
- In the device menu, double-click the side button to return to the home screen.
- In the device menu, press the center button to toggle the selected device.
- The screen turns off automatically after a period of inactivity.

## Device Menu

The device menu is designed for quick pocket use.

It shows:

- The previous device at the top.
- The selected device in the center.
- The next device at the bottom.

You can customize the entities, services, and icons directly from the substitutions section in the ESPHome YAML.

## Voice Assistant

Habbit Pocket uses the native Home Assistant voice assistant.

When the assistant starts listening, the device plays a short confirmation beep. After the command is executed, it plays another confirmation beep and returns to idle.

## Hardware

This project is built for the **M5StickS3 ESP32-S3 Mini IoT Dev Kit**.

- [AliExpress](https://s.click.aliexpress.com/e/_c3UTfFZx)
- [Amazon](https://amzn.to/4upCY6Y)
- [M5Stack Store](https://shop.m5stack.com/products/m5sticks3-esp32s3-mini-iot-dev-kit&ref=epuuxemo)

## ESPHome Components

The project uses custom ESPHome components for the M5StickS3 hardware features that need dedicated support:

- `m5sticks3_display`
- `m5sticks3_microphone`
- `m5sticks3_power`

These components are loaded from this repository through ESPHome `external_components`.

## Current Scope

This stable version focuses on:

- Display
- Buttons
- Backlight
- LVGL interface
- Confirmation beep
- Microphone
- Native Home Assistant voice assistant
- Customizable Home Assistant device menu

Battery readings, IR reception, and full media-player audio playback are intentionally not enabled in this baseline version.
