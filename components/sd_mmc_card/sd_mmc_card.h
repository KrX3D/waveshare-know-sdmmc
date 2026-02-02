#pragma once

#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include <vector>
#include <string>

namespace esphome {
namespace sd_mmc_card {

enum class CardType {
  UNKNOWN,
  MMC,
  SDSC,
  SDHC,
  SDXC
};

struct FileInfo {
  std::string path;
  size_t size;
  bool is_directory;
};

class SdMmcCard : public Component {
 public:
  void setup() override;
  void dump_config() override;

  // config
  void set_clk_pin(uint8_t pin) { clk_pin_ = pin; }
  void set_cmd_pin(uint8_t pin) { cmd_pin_ = pin; }
  void set_data0_pin(uint8_t pin) { d0_pin_ = pin; }
  void set_data1_pin(uint8_t pin) { d1_pin_ = pin; }
  void set_data2_pin(uint8_t pin) { d2_pin_ = pin; }
  void set_data3_pin(uint8_t pin) { d3_pin_ = pin; }
  void set_mode_1bit(bool v) { mode_1bit_ = v; }

  // file operations
  bool write_file(const std::string &path, const std::string &data);
  bool append_file(const std::string &path, const std::string &data);
  bool read_file(const std::string &path, std::string &out);
  bool delete_file(const std::string &path);

  bool create_directory(const std::string &path);
  bool remove_directory(const std::string &path);

  size_t file_size(const std::string &path);
  bool is_directory(const std::string &path);

  std::vector<std::string> list_directory(const std::string &path, uint8_t depth);
  std::vector<FileInfo> list_directory_file_info(const std::string &path, uint8_t depth);

  // space
  uint64_t total_space();
  uint64_t free_space();
  uint64_t used_space();

  CardType card_type() const { return card_type_; }

  // --- NEW: text sensor registration ---
  void register_card_type_text_sensor(text_sensor::TextSensor *ts) { card_type_sensor_ = ts; }
  void register_file_content_text_sensor(text_sensor::TextSensor *ts) { file_content_sensor_ = ts; }

  void publish_card_type(const std::string &type) {
    if (card_type_sensor_)
      card_type_sensor_->publish_state(type.c_str());
  }

  void publish_file_content(const std::string &content) {
    if (file_content_sensor_)
      file_content_sensor_->publish_state(content.c_str());
  }

 protected:
  void scan_dir_(const std::string &path, uint8_t depth, std::vector<FileInfo> &out);

  uint8_t clk_pin_{0}, cmd_pin_{0}, d0_pin_{0}, d1_pin_{0}, d2_pin_{0}, d3_pin_{0};
  bool mode_1bit_{false};

  CardType card_type_{CardType::UNKNOWN};

  // --- NEW: pointers to text sensors ---
  text_sensor::TextSensor *card_type_sensor_{nullptr};
  text_sensor::TextSensor *file_content_sensor_{nullptr};
};

}  // namespace sd_mmc_card
}  // namespace esphome
