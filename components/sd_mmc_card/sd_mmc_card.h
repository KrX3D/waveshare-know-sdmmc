#pragma once

#include "esphome/core/component.h"

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"

namespace esphome {
namespace sd_mmc_card {

class SdMmcCard : public Component {
 public:
  void set_clk_pin(int pin) { clk_pin_ = pin; }
  void set_cmd_pin(int pin) { cmd_pin_ = pin; }
  void set_data0_pin(int pin) { d0_pin_ = pin; }
  void set_data1_pin(int pin) { d1_pin_ = pin; }
  void set_data2_pin(int pin) { d2_pin_ = pin; }
  void set_data3_pin(int pin) { d3_pin_ = pin; }
  void set_mode_1bit(bool v) { mode_1bit_ = v; }
  void set_mount_point(const std::string &mp) { mount_point_ = mp; }

  void setup() override;
  void dump_config() override;

  bool write_file(const std::string &path, const std::string &data);
  bool read_file(const std::string &path, std::string &out);
  int64_t file_size(const std::string &path);
  std::vector<std::string> list_dir(const std::string &path);

 protected:
  int clk_pin_;
  int cmd_pin_;
  int d0_pin_;
  int d1_pin_;
  int d2_pin_;
  int d3_pin_;
  bool mode_1bit_{false};

  std::string mount_point_;
  sdmmc_card_t *card_{nullptr};
};

}  // namespace sd_mmc_card
}  // namespace esphome
