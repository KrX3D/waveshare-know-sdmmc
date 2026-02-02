#include "sd_mmc_card.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "ff.h"
#include <sys/stat.h>
#include <cstring>

namespace esphome {
namespace sd_mmc_card {

static const char *TAG = "sd_mmc_card";
static const char *MOUNT_POINT = "/sdcard";
static const char *FATFS_ROOT = "0:";

static constexpr char kFatfsSeparator = '/';

using FatfsDir = FF_DIR;
using FatfsFileInfo = FF_FILINFO;

static std::string fatfs_path_(const std::string &path) {
  if (path.empty()) {
    return std::string(FATFS_ROOT) + kFatfsSeparator;
  }
  if (path.front() == '/') {
    return std::string(FATFS_ROOT) + path;
  }
  return std::string(FATFS_ROOT) + kFatfsSeparator + path;
}

void SdMmcCard::setup() {
  sdmmc_host_t host = SDMMC_HOST_DEFAULT();
  sdmmc_slot_config_t slot = SDMMC_SLOT_CONFIG_DEFAULT();

  slot.clk = (gpio_num_t) clk_pin_;
  slot.cmd = (gpio_num_t) cmd_pin_;
  slot.d0  = (gpio_num_t) d0_pin_;
  slot.d1  = mode_1bit_ ? GPIO_NUM_NC : (gpio_num_t) d1_pin_;
  slot.d2  = mode_1bit_ ? GPIO_NUM_NC : (gpio_num_t) d2_pin_;
  slot.d3  = mode_1bit_ ? GPIO_NUM_NC : (gpio_num_t) d3_pin_;
  slot.width = mode_1bit_ ? 1 : 4;

  esp_vfs_fat_sdmmc_mount_config_t mount_cfg{};
  mount_cfg.format_if_mount_failed = false;
  mount_cfg.max_files = 5;

  esp_err_t err = esp_vfs_fat_sdmmc_mount(
      MOUNT_POINT, &host, &slot, &mount_cfg, &card_);

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Mount failed: %s", esp_err_to_name(err));
    mark_failed();
    return;
  }

  // Store card info
  if (card_) {
    switch (card_->ocr & SD_OCR_SDHC_CAP) {
      case 0: card_type_ = CardType::SDSC; break;
      default: card_type_ = CardType::SDHC; break;
    }
    card_freq_khz_ = card_->max_freq_khz;
  }

  ESP_LOGI(TAG, "Mounted at %s (%s-bit, %d kHz)",
           MOUNT_POINT, mode_1bit_ ? "1" : "4", card_freq_khz_);
  
  // Publish initial sensor values
  update_sensors_();
}

void SdMmcCard::dump_config() {
  ESP_LOGCONFIG(TAG, "SD MMC Card:");
  ESP_LOGCONFIG(TAG, "  Mount: %s", MOUNT_POINT);
  ESP_LOGCONFIG(TAG, "  Mode: %s-bit", mode_1bit_ ? "1" : "4");
  ESP_LOGCONFIG(TAG, "  Card Type: %s", card_type_string_().c_str());
  ESP_LOGCONFIG(TAG, "  Frequency: %d kHz", card_freq_khz_);
}

void SdMmcCard::loop() {
  // Update sensors periodically
  static uint32_t last_update = 0;
  uint32_t now = millis();
  if (now - last_update > 60000) {  // Update every 60 seconds
    update_sensors_();
    last_update = now;
  }
}

void SdMmcCard::update_sensors_() {
  if (total_space_sensor_)
    total_space_sensor_->publish_state(total_space());
  
  if (used_space_sensor_)
    used_space_sensor_->publish_state(used_space());
  
  if (free_space_sensor_)
    free_space_sensor_->publish_state(free_space());
  
  if (file_size_sensor_ && !file_size_path_.empty())
    file_size_sensor_->publish_state(file_size(file_size_path_));
  
  if (card_type_sensor_)
    card_type_sensor_->publish_state(card_type_string_());
}

std::string SdMmcCard::card_type_string_() {
  switch (card_type_) {
    case CardType::MMC: return "MMC";
    case CardType::SDSC: return "SDSC";
    case CardType::SDHC: return "SDHC";
    case CardType::SDXC: return "SDXC";
    default: return "UNKNOWN";
  }
}

/* ---------- file ops ---------- */

bool SdMmcCard::write_file(const std::string &path, const std::string &data) {
  FILE *f = fopen((std::string(MOUNT_POINT) + path).c_str(), "w");
  if (!f) return false;
  fwrite(data.data(), 1, data.size(), f);
  fclose(f);
  
  // Update file size sensor if monitoring this file
  if (file_size_sensor_ && file_size_path_ == path)
    file_size_sensor_->publish_state(file_size(path));
  
  return true;
}

bool SdMmcCard::append_file(const std::string &path, const std::string &data) {
  FILE *f = fopen((std::string(MOUNT_POINT) + path).c_str(), "a");
  if (!f) return false;
  fwrite(data.data(), 1, data.size(), f);
  fclose(f);
  
  // Update file size sensor if monitoring this file
  if (file_size_sensor_ && file_size_path_ == path)
    file_size_sensor_->publish_state(file_size(path));
  
  return true;
}

bool SdMmcCard::read_file(const std::string &path, std::string &out) {
  FILE *f = fopen((std::string(MOUNT_POINT) + path).c_str(), "r");
  if (!f) return false;
  char buf[256];
  out.clear();
  while (fgets(buf, sizeof(buf), f))
    out += buf;
  fclose(f);
  return true;
}

bool SdMmcCard::delete_file(const std::string &path) {
  bool result = unlink((std::string(MOUNT_POINT) + path).c_str()) == 0;
  
  // Update file size sensor if monitoring this file
  if (result && file_size_sensor_ && file_size_path_ == path)
    file_size_sensor_->publish_state(0);
  
  return result;
}

/* ---------- dirs ---------- */

bool SdMmcCard::create_directory(const std::string &path) {
  FRESULT result = f_mkdir(fatfs_path_(path).c_str());
  return result == FR_OK || result == FR_EXIST;
}

bool SdMmcCard::remove_directory(const std::string &path) {
  return f_unlink(fatfs_path_(path).c_str()) == FR_OK;
}

bool SdMmcCard::is_directory(const std::string &path) {
  FatfsFileInfo info{};
  if (f_stat(fatfs_path_(path).c_str(), &info) != FR_OK)
    return false;
  return (info.fattrib & AM_DIR) != 0;
}

size_t SdMmcCard::file_size(const std::string &path) {
  FatfsFileInfo info{};
  if (f_stat(fatfs_path_(path).c_str(), &info) != FR_OK)
    return 0;
  return info.fsize;
}

/* ---------- list directory ---------- */

std::vector<std::string> SdMmcCard::list_directory(const std::string &path, uint8_t depth) {
  std::vector<FileInfo> file_infos;
  scan_dir_(path, depth, file_infos);
  
  std::vector<std::string> result;
  for (const auto &info : file_infos) {
    result.push_back(info.path);
  }
  return result;
}

std::vector<FileInfo> SdMmcCard::list_directory_file_info(const std::string &path, uint8_t depth) {
  std::vector<FileInfo> result;
  scan_dir_(path, depth, result);
  return result;
}

void SdMmcCard::scan_dir_(const std::string &path, uint8_t depth, std::vector<FileInfo> &out) {
  if (depth == 0) return;
  
  std::string full_path = fatfs_path_(path);
  ESP_LOGD(TAG, "Scanning directory: %s", full_path.c_str());

  FatfsDir dir{};
  FRESULT result = f_opendir(&dir, full_path.c_str());
  if (result != FR_OK) {
    ESP_LOGE(TAG, "Failed to open directory: %s (fatfs err: %d)", full_path.c_str(), result);
    return;
  }

  FatfsFileInfo file_info{};
  int count = 0;
  while (true) {
    result = f_readdir(&dir, &file_info);
    if (result != FR_OK || file_info.fname[0] == '\0')
      break;
    if (strcmp(file_info.fname, ".") == 0 || strcmp(file_info.fname, "..") == 0)
      continue;
    
    count++;
    std::string entry_path = path;
    if (!entry_path.empty() && entry_path.back() != kFatfsSeparator)
      entry_path += kFatfsSeparator;
    entry_path += file_info.fname;

    bool is_dir = (file_info.fattrib & AM_DIR) != 0;
    out.push_back(FileInfo{entry_path, static_cast<size_t>(file_info.fsize), is_dir});
    ESP_LOGD(TAG, "  Found: %s (%s, %d bytes)", file_info.fname, is_dir ? "DIR" : "FILE",
             static_cast<int>(file_info.fsize));

    if (is_dir && depth > 1) {
      scan_dir_(entry_path, depth - 1, out);
    }
  }
  ESP_LOGD(TAG, "Total entries found in %s: %d", path.c_str(), count);
  f_closedir(&dir);
}

/* ---------- space ---------- */

uint64_t SdMmcCard::total_space() {
  FATFS *fs;
  DWORD free_clust;
  f_getfree("0:", &free_clust, &fs);
  return (uint64_t) fs->n_fatent * fs->csize * 512;
}

uint64_t SdMmcCard::free_space() {
  FATFS *fs;
  DWORD free_clust;
  f_getfree("0:", &free_clust, &fs);
  return (uint64_t) free_clust * fs->csize * 512;
}

uint64_t SdMmcCard::used_space() {
  return total_space() - free_space();
}

}  // namespace sd_mmc_card
}  // namespace esphome
