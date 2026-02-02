#include "sd_card_bsp.h"

#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "driver/gpio.h"

namespace sd_card_bsp {

static SdCardBSP *instance = nullptr;

SdCardBSP::SdCardBSP() {
  mounted_ = false;
  card_handle_ = nullptr;
  mode_1bit_ = false;
}

SdCardBSP *get_sd_card_bsp() {
  if (!instance) instance = new SdCardBSP();
  return instance;
}

bool SdCardBSP::init(bool mode_1bit,
                     gpio_num_t clk_pin,
                     gpio_num_t cmd_pin,
                     gpio_num_t d0_pin,
                     gpio_num_t d1_pin,
                     gpio_num_t d2_pin,
                     gpio_num_t d3_pin,
                     const std::string &mount_point) {
  this->mode_1bit_ = mode_1bit;
  this->mount_point_ = mount_point;

  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
    .format_if_mount_failed = false,
    .max_files = 5,
    .allocation_unit_size = 512,
  };

  sdmmc_host_t host = SDMMC_HOST_DEFAULT();
  host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;

  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
  slot_config.width = mode_1bit ? 1 : 4;
  slot_config.clk = clk_pin;
  slot_config.cmd = cmd_pin;
  slot_config.d0 = d0_pin;
  slot_config.d1 = d1_pin;
  slot_config.d2 = d2_pin;
  slot_config.d3 = d3_pin;

  sdmmc_card_t *card = nullptr;
  esp_err_t err = esp_vfs_fat_sdmmc_mount(mount_point.c_str(), &host, &slot_config, &mount_config, &card);
  if (err != ESP_OK) {
    ESP_LOGE("sd_card_bsp", "esp_vfs_fat_sdmmc_mount failed: %d", err);
    this->mounted_ = false;
    this->card_handle_ = nullptr;
    return false;
  }

  sdmmc_card_print_info(stdout, card);
  this->card_handle_ = (void *)card;
  this->mounted_ = true;
  return true;
}

static std::string normalize_path(const std::string &p, const std::string &mount_point) {
  if (p.size() > 0 && p[0] == '/') return p; // absolute
  return mount_point + "/" + p;
}

bool SdCardBSP::write_file(const std::string &path, const std::vector<uint8_t> &data) {
  if (!mounted_ || card_handle_ == nullptr) return false;
  const std::string p = normalize_path(path, this->mount_point_);
  FILE *f = fopen(p.c_str(), "wb");
  if (!f) {
    ESP_LOGE("sd_card_bsp", "Failed to open file for writing: %s (errno %d)", p.c_str(), errno);
    return false;
  }
  size_t w = fwrite(data.data(), 1, data.size(), f);
  fclose(f);
  return (w == data.size());
}

bool SdCardBSP::append_file(const std::string &path, const std::vector<uint8_t> &data) {
  if (!mounted_ || card_handle_ == nullptr) return false;
  const std::string p = normalize_path(path, this->mount_point_);
  FILE *f = fopen(p.c_str(), "ab");
  if (!f) {
    ESP_LOGE("sd_card_bsp", "Failed to open file for append: %s (errno %d)", p.c_str(), errno);
    return false;
  }
  size_t w = fwrite(data.data(), 1, data.size(), f);
  fclose(f);
  return (w == data.size());
}

std::vector<uint8_t> SdCardBSP::read_file(const std::string &path) {
  std::vector<uint8_t> out;
  if (!mounted_ || card_handle_ == nullptr) return out;
  const std::string p = normalize_path(path, this->mount_point_);
  FILE *f = fopen(p.c_str(), "rb");
  if (!f) {
    ESP_LOGE("sd_card_bsp", "Failed to open file for read: %s (errno %d)", p.c_str(), errno);
    return out;
  }
  fseek(f, 0, SEEK_END);
  long sz = ftell(f);
  fseek(f, 0, SEEK_SET);
  if (sz <= 0) {
    fclose(f);
    return out;
  }
  out.resize(sz);
  size_t r = fread(out.data(), 1, sz, f);
  fclose(f);
  if (r != (size_t)sz) {
    ESP_LOGE("sd_card_bsp", "Short read: %d vs %d", (int)r, (int)sz);
    out.clear();
  }
  return out;
}

ssize_t SdCardBSP::file_size(const std::string &path) {
  if (!mounted_ || card_handle_ == nullptr) return -1;
  const std::string p = normalize_path(path, this->mount_point_);
  struct stat st;
  if (stat(p.c_str(), &st) != 0) {
    ESP_LOGE("sd_card_bsp", "stat failed %s (errno %d)", p.c_str(), errno);
    return -1;
  }
  return (ssize_t)st.st_size;
}

bool SdCardBSP::delete_file(const std::string &path) {
  if (!mounted_ || card_handle_ == nullptr) return false;
  const std::string p = normalize_path(path, this->mount_point_);
  int r = remove(p.c_str());
  if (r != 0) {
    ESP_LOGE("sd_card_bsp", "remove failed %s (errno %d)", p.c_str(), errno);
    return false;
  }
  return true;
}

std::vector<std::string> SdCardBSP::list_directory(const std::string &path, uint8_t depth) {
  std::vector<std::string> out;
  if (!mounted_ || card_handle_ == nullptr) return out;
  const std::string p = normalize_path(path, this->mount_point_);
  DIR *d = opendir(p.c_str());
  if (!d) {
    ESP_LOGE("sd_card_bsp", "opendir failed %s (errno %d)", p.c_str(), errno);
    return out;
  }
  struct dirent *ent;
  while ((ent = readdir(d)) != nullptr) {
    std::string name = ent->d_name;
    if (name == "." || name == "..") continue;
    out.push_back(p + "/" + name);
  }
  closedir(d);
  return out;
}

std::vector<SdCardBSP::FileInfo> SdCardBSP::list_directory_file_info(const std::string &path, uint8_t depth) {
  std::vector<FileInfo> out;
  if (!mounted_ || card_handle_ == nullptr) return out;
  const std::string p = normalize_path(path, this->mount_point_);
  DIR *d = opendir(p.c_str());
  if (!d) {
    ESP_LOGE("sd_card_bsp", "opendir failed %s (errno %d)", p.c_str(), errno);
    return out;
  }
  struct dirent *ent;
  while ((ent = readdir(d)) != nullptr) {
    std::string name = ent->d_name;
    if (name == "." || name == "..") continue;
    std::string full = p + "/" + name;
    struct stat st;
    if (stat(full.c_str(), &st) != 0) {
      ESP_LOGE("sd_card_bsp", "stat failed for %s (errno %d)", full.c_str(), errno);
      continue;
    }
    FileInfo fi{full, (size_t)st.st_size, (bool)S_ISDIR(st.st_mode)};
    out.push_back(fi);
  }
  closedir(d);
  return out;
}

bool SdCardBSP::create_directory(const std::string &path) {
  if (!mounted_ || card_handle_ == nullptr) return false;
  const std::string p = normalize_path(path, this->mount_point_);
  int r = mkdir(p.c_str(), 0777);
  if (r != 0) {
    ESP_LOGE("sd_card_bsp", "mkdir failed %s (errno %d)", p.c_str(), errno);
    return false;
  }
  return true;
}

bool SdCardBSP::remove_directory(const std::string &path) {
  if (!mounted_ || card_handle_ == nullptr) return false;
  const std::string p = normalize_path(path, this->mount_point_);
  int r = rmdir(p.c_str());
  if (r != 0) {
    ESP_LOGE("sd_card_bsp", "rmdir failed %s (errno %d)", p.c_str(), errno);
    return false;
  }
  return true;
}

bool SdCardBSP::is_directory(const std::string &path) {
  if (!mounted_ || card_handle_ == nullptr) return false;
  const std::string p = normalize_path(path, this->mount_point_);
  struct stat st;
  if (stat(p.c_str(), &st) != 0) return false;
  return (bool)S_ISDIR(st.st_mode);
}

float SdCardBSP::card_capacity_gb() {
  if (!mounted_ || card_handle_ == nullptr) return 0.0f;
  sdmmc_card_t *card = (sdmmc_card_t *)this->card_handle_;
  if (!card) return 0.0f;
  // capacity is in sectors * sector size (see your Arduino code: capacity/2048/1024 gave GiB)
  uint64_t capacity = (uint64_t)card->csd.capacity; // number of sectors maybe
  // guard
  if (capacity == 0) return 0.0f;
  // using same formula as your Arduino code (note: may vary with card)
  float g = ((float)capacity) / 2048.0f / 1024.0f;
  return g;
}

void SdCardBSP::loop() {
  // publish sensors if set
  if (!mounted_) return;
  if (total_space_sensor_ || free_space_sensor_ || used_space_sensor_) {
    // Attempt to compute using statfs on mount point
    struct statvfs st;
    if (statvfs(this->mount_point_.c_str(), &st) == 0) {
      uint64_t total = (uint64_t)st.f_blocks * (uint64_t)st.f_frsize;
      uint64_t free = (uint64_t)st.f_bfree * (uint64_t)st.f_frsize;
      uint64_t used = total - free;
      if (total_space_sensor_) total_space_sensor_->publish_state((float)total);
      if (free_space_sensor_) free_space_sensor_->publish_state((float)free);
      if (used_space_sensor_) used_space_sensor_->publish_state((float)used);
    }
  }

  if (card_type_text_ && card_handle_) {
    sdmmc_card_t *card = (sdmmc_card_t *)this->card_handle_;
    std::string type = sdmmc_card_get_type(card); // helper from sdmmc? fallback to "SD"
    // not all builds expose sdmmc_card_get_type; guard
    card_type_text_->publish_state(type.c_str());
  }
}

}  // namespace sd_card_bsp
