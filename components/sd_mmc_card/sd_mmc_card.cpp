#include "sd_mmc_card.h"
#include "esphome/core/log.h"

#include <sys/stat.h>
#include <dirent.h>

namespace esphome {
namespace sd_mmc_card {

static const char *TAG = "sd_mmc_card";

void SdMmcCard::setup() {
  ESP_LOGI(TAG, "Mounting SD card at %s", mount_point_.c_str());

  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = false,
      .max_files = 5,
      .allocation_unit_size = 512,
  };

  sdmmc_host_t host = SDMMC_HOST_DEFAULT();
  host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;

  sdmmc_slot_config_t slot = SDMMC_SLOT_CONFIG_DEFAULT();
  slot.width = mode_1bit_ ? 1 : 4;
  slot.clk = (gpio_num_t) clk_pin_;
  slot.cmd = (gpio_num_t) cmd_pin_;
  slot.d0  = (gpio_num_t) d0_pin_;
  if (!mode_1bit_) {
    slot.d1 = (gpio_num_t) d1_pin_;
    slot.d2 = (gpio_num_t) d2_pin_;
    slot.d3 = (gpio_num_t) d3_pin_;
  }

  esp_err_t err = esp_vfs_fat_sdmmc_mount(
      mount_point_.c_str(), &host, &slot, &mount_config, &card_);

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "SD mount failed: %s", esp_err_to_name(err));
    mark_failed();
    return;
  }

  sdmmc_card_print_info(stdout, card_);
}

void SdMmcCard::dump_config() {
  ESP_LOGCONFIG(TAG, "SD MMC Card:");
  ESP_LOGCONFIG(TAG, "  Mount point: %s", mount_point_.c_str());
  ESP_LOGCONFIG(TAG, "  Mode: %s", mode_1bit_ ? "1-bit" : "4-bit");
}

bool SdMmcCard::write_file(const std::string &path, const std::string &data) {
  std::string full = mount_point_ + path;
  FILE *f = fopen(full.c_str(), "w");
  if (!f) {
    ESP_LOGE(TAG, "Write failed: %s", full.c_str());
    return false;
  }
  fwrite(data.data(), 1, data.size(), f);
  fclose(f);
  return true;
}

bool SdMmcCard::read_file(const std::string &path, std::string &out) {
  std::string full = mount_point_ + path;
  FILE *f = fopen(full.c_str(), "rb");
  if (!f)
    return false;

  fseek(f, 0, SEEK_END);
  size_t len = ftell(f);
  fseek(f, 0, SEEK_SET);

  out.resize(len);
  fread(out.data(), 1, len, f);
  fclose(f);
  return true;
}

int64_t SdMmcCard::file_size(const std::string &path) {
  std::string full = mount_point_ + path;
  struct stat st;
  if (stat(full.c_str(), &st) != 0)
    return -1;
  return st.st_size;
}

std::vector<std::string> SdMmcCard::list_dir(const std::string &path) {
  std::vector<std::string> out;
  std::string full = mount_point_ + path;
  DIR *dir = opendir(full.c_str());
  if (!dir)
    return out;

  struct dirent *e;
  while ((e = readdir(dir)) != nullptr) {
    out.emplace_back(e->d_name);
  }
  closedir(dir);
  return out;
}

}  // namespace sd_mmc_card
}  // namespace esphome
