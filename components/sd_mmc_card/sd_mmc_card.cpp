#include "sd_mmc_card.h"
#include "esphome/core/log.h"

#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "ff.h"
#include <sys/stat.h>
#include <dirent.h>

namespace esphome {
namespace sd_mmc_card {

static const char *TAG = "sd_mmc_card";
static const char *MOUNT_POINT = "/sdcard";

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

  sdmmc_card_t *card;
  esp_err_t err = esp_vfs_fat_sdmmc_mount(
      MOUNT_POINT, &host, &slot, &mount_cfg, &card);

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Mount failed: %s", esp_err_to_name(err));
    mark_failed();
    return;
  }

  switch (card->ocr & SD_OCR_SDHC_CAP) {
    case 0: card_type_ = CardType::SDSC; break;
    default: card_type_ = CardType::SDHC; break;
  }

  ESP_LOGI(TAG, "Mounted at %s (%s-bit)",
           MOUNT_POINT, mode_1bit_ ? "1" : "4");
  
  // Publish initial sensor values
  update_sensors_();
}

void SdMmcCard::dump_config() {
  ESP_LOGCONFIG(TAG, "SD MMC Card:");
  ESP_LOGCONFIG(TAG, "  Mount: %s", MOUNT_POINT);
  ESP_LOGCONFIG(TAG, "  Mode: %s-bit", mode_1bit_ ? "1" : "4");
  ESP_LOGCONFIG(TAG, "  Card Type: %s", card_type_string_().c_str());
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
  return mkdir((std::string(MOUNT_POINT) + path).c_str(), 0775) == 0;
}

bool SdMmcCard::remove_directory(const std::string &path) {
  return rmdir((std::string(MOUNT_POINT) + path).c_str()) == 0;
}

bool SdMmcCard::is_directory(const std::string &path) {
  struct stat st{};
  if (stat((std::string(MOUNT_POINT) + path).c_str(), &st) != 0)
    return false;
  return S_ISDIR(st.st_mode);
}

size_t SdMmcCard::file_size(const std::string &path) {
  struct stat st{};
  if (stat((std::string(MOUNT_POINT) + path).c_str(), &st) != 0)
    return 0;
  return st.st_size;
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
  
  std::string full_path = std::string(MOUNT_POINT) + path;
  DIR *dir = opendir(full_path.c_str());
  if (!dir) return;
  
  struct dirent *entry;
  while ((entry = readdir(dir)) != nullptr) {
    // Skip . and ..
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;
    
    std::string entry_path = path;
    if (!entry_path.empty() && entry_path.back() != '/')
      entry_path += "/";
    entry_path += entry->d_name;
    
    struct stat st{};
    std::string full_entry = std::string(MOUNT_POINT) + entry_path;
    if (stat(full_entry.c_str(), &st) == 0) {
      bool is_dir = S_ISDIR(st.st_mode);
      out.push_back(FileInfo{entry_path, (size_t)st.st_size, is_dir});
      
      if (is_dir && depth > 1) {
        scan_dir_(entry_path, depth - 1, out);
      }
    }
  }
  closedir(dir);
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