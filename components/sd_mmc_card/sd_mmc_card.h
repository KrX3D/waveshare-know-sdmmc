#pragma once

#include "esphome/core/component.h"
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

  // file ops
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

 protected:
  void scan_dir_(const std::string &path, uint8_t depth,
                 std::vector<FileInfo> &out);

  uint8_t clk_pin_{0}, cmd_pin_{0}, d0_pin_{0}, d1_pin_{0}, d2_pin_{0}, d3_pin_{0};
  bool mode_1bit_{false};

  CardType card_type_{CardType::UNKNOWN};
};

}  // namespace sd_mmc_card
}  // namespace esphome
