# SD/MMC Card Component for ESPHome

This repository provides a custom `sd_mmc_card` component for ESPHome using
ESP-IDF + FatFs. It mounts an SD/MMC card and exposes file, directory, and
capacity helpers to automations, along with sensors and text sensors.

This component was developed for the Waveshare ESP32-S3 Knob Touch LCD 1.8"
board and its SD/MMC wiring, but it can be used on any compatible ESP32 target:
https://www.waveshare.com/esp32-s3-knob-touch-lcd-1.8.htm

## Features
- Mount SD/MMC via `esp_vfs_fat_sdmmc_mount()` on ESP32 (1-bit or 4-bit mode).
- File helpers: write, append, read, delete, file size.
- Directory helpers: list, list with file info, create/remove directory.
- Sensors: total space, used space, free space, **frequency** (kHz), file size.
- Text sensors: card type, file content.

## Installation
### Option A: external_components (recommended)
```yaml
external_components:
  - source:
      type: git
      url: https://github.com/KrX3D/waveshare-know-sdmmc
      ref: main
    refresh: 0s
    components: [sd_mmc_card]
```

### Option B: custom_components
Copy the `components/sd_mmc_card` folder into your ESPHome project’s
`custom_components/` directory.

```
custom_components/
  sd_mmc_card/
    __init__.py
    sensor.py
    text_sensor.py
    sd_mmc_card.h
    sd_mmc_card.cpp
```

## Configuration

### Component
```yaml
sd_mmc_card:
  id: esp_sd_card
  mode_1bit: false       # optional (default false)
  clk_pin: GPIO4         # required
  cmd_pin: GPIO3         # required
  data0_pin: GPIO5       # required
  data1_pin: GPIO6       # required when mode_1bit: false
  data2_pin: GPIO42      # required when mode_1bit: false
  data3_pin: GPIO2       # required when mode_1bit: false
```

#### 1-bit mode example
```yaml
sd_mmc_card:
  id: esp_sd_card
  mode_1bit: true
  clk_pin: GPIO4
  cmd_pin: GPIO3
  data0_pin: GPIO5
```

### Sensors
```yaml
sensor:
  - platform: sd_mmc_card
    sd_mmc_card_id: esp_sd_card
    type: total_space
    name: "SD Total Space"
    unit_of_measurement: "MB"
    accuracy_decimals: 2
    filters:
      - lambda: return x / 1024.0 / 1024.0;

  - platform: sd_mmc_card
    sd_mmc_card_id: esp_sd_card
    type: used_space
    name: "SD Used Space"
    unit_of_measurement: "MB"
    accuracy_decimals: 2
    filters:
      - lambda: return x / 1024.0 / 1024.0;

  - platform: sd_mmc_card
    sd_mmc_card_id: esp_sd_card
    type: free_space
    name: "SD Free Space"
    unit_of_measurement: "MB"
    accuracy_decimals: 2
    filters:
      - lambda: return x / 1024.0 / 1024.0;

  - platform: sd_mmc_card
    sd_mmc_card_id: esp_sd_card
    type: frequency
    name: "SD Frequency"
    unit_of_measurement: "kHz"
    accuracy_decimals: 0

  - platform: sd_mmc_card
    sd_mmc_card_id: esp_sd_card
    type: file_size
    path: "/test.txt"
    name: "Test.txt Size"
    unit_of_measurement: "B"
    accuracy_decimals: 0
```

#### Sensor reference
- **total_space**: Total card capacity in bytes.
- **used_space**: Used bytes on the card.
- **free_space**: Free bytes on the card.
- **frequency**: SD/MMC bus frequency in kHz.
- **file_size**: Size in bytes of the file at `path`.

### Text sensors
```yaml
text_sensor:
  - platform: sd_mmc_card
    sd_mmc_card_id: esp_sd_card
    type: sd_card_type
    name: "SD Card Type"

  - platform: sd_mmc_card
    sd_mmc_card_id: esp_sd_card
    type: file_content
    name: "SD File Content"
```

#### Text sensor reference
- **sd_card_type**: Card type (MMC/SDSC/SDHC/SDXC/UNKNOWN).
- **file_content**: String contents of the last read file when you publish it.

### Methods you can call from lambdas
```cpp
id(esp_sd_card).write_file("/test.txt", "data");
id(esp_sd_card).append_file("/test.txt", "\nline");
id(esp_sd_card).read_file("/test.txt", out);
id(esp_sd_card).delete_file("/test.txt");
id(esp_sd_card).format_card();
id(esp_sd_card).create_directory("/testdir");
id(esp_sd_card).remove_directory("/testdir");
id(esp_sd_card).list_directory("/", 2);
id(esp_sd_card).list_directory_file_info("/", 2);
id(esp_sd_card).file_size("/test.txt");
```

#### Lambda helper reference
- **write_file**: Overwrite/create a file with data.
- **append_file**: Append data to an existing file.
- **read_file**: Read file contents into a string.
- **delete_file**: Remove a file from the card.
- **format_card**: Format the card (remount required afterward).
- **create_directory/remove_directory**: Manage directories.
- **list_directory/list_directory_file_info**: Enumerate directory entries.
- **file_size**: Returns the size of a file in bytes.

### Format button example
```yaml
button:
  - platform: template
    name: "SD Format Card"
    on_press:
      then:
        - lambda: |-
            if (id(esp_sd_card).format_card()) {
              ESP_LOGW("sd_card", "Format complete; remount required");
            } else {
              ESP_LOGE("sd_card", "Format failed");
            }
```

## Full Example
See [example.yaml](example.yaml) for a complete working configuration including
buttons and sensors.

## Notes
- Paths are relative to the SD mount, so use `/` prefixes (e.g., `/test.txt`).
- `sd_mmc_card_id` is required for sensors and text sensors to reference the
  component instance (ESPHome cannot infer the component ID automatically).
- `frequency` is reported in kHz.
