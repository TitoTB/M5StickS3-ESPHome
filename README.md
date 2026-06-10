# M5StickS3 ESPHome

ESPHome integration for the M5Stack StickS3 ESP32-S3 Mini IoT Dev Kit, designed for Home Assistant.

This project provides a working baseline firmware with custom ESPHome components for the parts of the StickS3 that are not covered well enough by native ESPHome components.

## Current Status

This version includes:

- M5StickS3 display support through a custom ESPHome component.
- LVGL interface with a home screen and a device menu.
- Screen backlight exposed as a Home Assistant light entity.
- Blue button and side button exposed as GPIO binary sensors.
- Internal confirmation beep through the StickS3 speaker.
- Microphone support through a custom M5Unified / `M5.Mic` ESPHome component.
- Native Home Assistant voice assistant support.
- Voice assistant activation with a long press on the blue button.
- Confirmation beep when the assistant starts listening.
- Confirmation beep when a command is executed.
- Device menu navigation from the side button.
- Device toggle from the blue button while the device menu is open.
- Screen sleep with backlight off.
- Double-click wake gesture on the blue button.

IR reception, battery readings, and media-player audio playback are intentionally not enabled in this stable baseline. They were left out to keep the display, buttons, beep, LVGL interface, and native Home Assistant assistant flow reliable.

## How It Works

The firmware uses:

- `m5sticks3_display` for the StickS3 screen.
- `m5sticks3_power` for power-related M5Stack initialization and the internal beep.
- `m5sticks3_microphone` for microphone capture through M5Unified.
- ESPHome `voice_assistant` for the native Home Assistant Assist pipeline.
- ESPHome `lvgl` for the on-device interface.

The home screen shows the project icon, date, time, and title.

The device menu shows three device icons:

- Previous device at the top.
- Current device in the center.
- Next device at the bottom.

From the device menu:

- Press the side button to move to the next device.
- Short press the blue button to toggle the selected device.

When the screen is asleep:

- A double click on the blue button wakes the screen.
- Long press actions are ignored so the assistant is not started accidentally.

## Usage

1. Copy `stick-s3.yaml` into ESPHome Builder.
2. Create a new ESPHome device normally so ESPHome can generate your own API, OTA, Wi-Fi, and fallback hotspot values.
3. Define these ESPHome secrets:

```yaml
wifi_ssid: "your_wifi_name"
wifi_password: "your_wifi_password"
```

4. Adjust the substitutions at the top of `stick-s3.yaml`:

```yaml
name: stick-s3
friendly_name: Stick S3
idle_time: 20s

devices_count: "4"

device_0_entity: "light.example"
device_0_service: "light.toggle"
device_0_icon: "mdi:ceiling-light"
```

5. Replace the example device entities, services, and icons with your own Home Assistant entities.

## External Components

The YAML loads the custom components from this repository:

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/TitoTB/M5StickS3-ESPHome.git
      ref: main
      path: components
    components:
      - m5sticks3_display
      - m5sticks3_microphone
      - m5sticks3_power
    refresh: 1days
```

`refresh: 1days` is the stable setting. During active component development you can temporarily change it to `0s` to force ESPHome to fetch the latest repository version on every compile.

## Notes

- Keep speaker volume and beep usage conservative when running on battery.
- Battery readings are not part of the stable baseline.
- IR reception is not part of the stable baseline.
- Full media-player playback is not part of the stable baseline.
- The current target is a reliable pocket remote and native Home Assistant voice assistant device.
