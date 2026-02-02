# sd_card_bsp — ESPHome SD/MMC minimal BSP component

A lightweight ESPHome custom component that mounts an SD/MMC card using ESP-IDF calls
and exposes simple file and directory helpers to lambdas and the rest of ESPHome.

## Features
- Mount SD/MMC via `esp_vfs_fat_sdmmc_mount()` with configurable pins (1-bit or 4-bit).
- File operations: write, append, read, delete.
- Directory operations: list, list with file info, create/remove directory, is_directory.
- File size and card capacity helper.
- Optional sensor publishing (total/free/used space, card type).
- Safe read semantics (use `file_size()` before `read_file()` to avoid out-of-memory).

## Files
- `sd_card_bsp.h` / `sd_card_bsp.cpp` : component implementation
- Example usage shown in `example.yaml` below.

## Installation
1. Create folder: `custom_components/sd_card_bsp/`
2. Copy `sd_card_bsp.h`, `sd_card_bsp.cpp`, and this README into the folder.
3. In your ESPHome YAML, include the header and call `init()` from `on_boot` (example below).

## Example YAML
```yaml
esphome:
  name: smartknob
  includes:
    - custom_components/sd_card_bsp/sd_card_bsp.h

esp32:
  framework:
    type: esp-idf
    sdkconfig_options:
      CONFIG_FATFS_LFN_STACK: "y"
      CONFIG_FATFS_FS_LOCK: "y"
      CONFIG_FATFS_PER_FILE_CACHE: "y"
      CONFIG_FATFS_USE_FASTSEEK: "y"
      CONFIG_FATFS_MAX_LFN: "255"

logger:
  level: DEBUG

text_sensor:
  - platform: template
    name: "SD Last Read"
    id: sd_file_content

# Optional sensors (wired up in on_boot)
sensor:
  - platform: template
    name: "SD Used Bytes"
    id: sd_used_bytes

  - platform: template
    name: "SD Total Bytes"
    id: sd_total_bytes

  - platform: template
    name: "SD Free Bytes"
    id: sd_free_bytes

# initialize and wire the component at boot
on_boot:
  then:
    - lambda: |-
        using namespace sd_card_bsp;
        auto sd = get_sd_card_bsp();
        // init(mode_1bit, clk, cmd, d0, d1, d2, d3, mount_point)
        sd->init(false, (gpio_num_t)4, (gpio_num_t)3, (gpio_num_t)5, (gpio_num_t)6, (gpio_num_t)42, (gpio_num_t)2, "/sdcard");
        // optionally attach sensors
        sd->set_used_space_sensor(id(sd_used_bytes));
        sd->set_total_space_sensor(id(sd_total_bytes));
        sd->set_free_space_sensor(id(sd_free_bytes));
        sd->set_card_type_text_sensor(id(sd_file_content)); // or use separate text sensor
