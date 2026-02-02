# SD/MMC Card Component for ESPHome

This repository provides a custom `sd_mmc_card` component for ESPHome using
ESP-IDF + FatFs. It mounts an SD/MMC card and exposes file, directory, and
capacity helpers to automations, along with sensors and text sensors.

## Features
- Mount SD/MMC via `esp_vfs_fat_sdmmc_mount()` on ESP32 (1-bit or 4-bit mode).
- File helpers: write, append, read, delete, file size.
- Directory helpers: list, list with file info, create/remove directory.
- Sensors: total space, used space, free space, **frequency** (kHz), file size.
- Text sensors: card type, file content.

## Installation
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
  mode_1bit: false
  clk_pin: GPIO4
  cmd_pin: GPIO3
  data0_pin: GPIO5
  data1_pin: GPIO6
  data2_pin: GPIO42
  data3_pin: GPIO2
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

### Methods you can call from lambdas
```cpp
id(esp_sd_card).write_file("/test.txt", "data");
id(esp_sd_card).append_file("/test.txt", "\nline");
id(esp_sd_card).read_file("/test.txt", out);
id(esp_sd_card).delete_file("/test.txt");
id(esp_sd_card).create_directory("/testdir");
id(esp_sd_card).remove_directory("/testdir");
id(esp_sd_card).list_directory("/", 2);
id(esp_sd_card).list_directory_file_info("/", 2);
id(esp_sd_card).file_size("/test.txt");
```

## Full Example
See [example.yaml](example.yaml) for a complete working configuration including
buttons and sensors.

## Notes
- Paths are relative to the SD mount, so use `/` prefixes (e.g., `/test.txt`).
- `frequency` is reported in kHz.
