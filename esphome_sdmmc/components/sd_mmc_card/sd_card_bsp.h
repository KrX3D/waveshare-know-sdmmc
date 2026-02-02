#pragma once
#include <vector>
#include <string>
#include <stdint.h>
#include "esphome.h"

#ifdef __cplusplus
extern "C" {
#endif
// ESP-IDF headers referenced by the .cpp (included there)
#ifdef __cplusplus
}
#endif

namespace sd_card_bsp {

class SdCardBSP : public Component {
 public:
  SdCardBSP();

  // Initialize / mount (call from on_boot)
  // mode_1bit = true for 1-bit (SPI-like), false for 4-bit
  bool init(bool mode_1bit,
            gpio_num_t clk_pin,
            gpio_num_t cmd_pin,
            gpio_num_t d0_pin,
            gpio_num_t d1_pin,
            gpio_num_t d2_pin,
            gpio_num_t d3_pin,
            const std::string &mount_point = "/sdcard");

  // Basic file ops
  bool write_file(const std::string &path, const std::vector<uint8_t> &data);
  bool append_file(const std::string &path, const std::vector<uint8_t> &data);
  std::vector<uint8_t> read_file(const std::string &path);
  ssize_t file_size(const std::string &path); // -1 on failure
  bool delete_file(const std::string &path);

  // Directory ops
  std::vector<std::string> list_directory(const std::string &path, uint8_t depth = 1);
  struct FileInfo { std::string path; size_t size; bool is_directory; };
  std::vector<FileInfo> list_directory_file_info(const std::string &path, uint8_t depth = 1);
  bool create_directory(const std::string &path);
  bool remove_directory(const std::string &path);
  bool is_directory(const std::string &path);

  // Card info
  float card_capacity_gb(); // 0.0 if not present
  bool mounted() const { return this->mounted_; }

  // Helpers to attach sensors (optional)
  void set_used_space_sensor(Sensor *s) { used_space_sensor_ = s; }
  void set_total_space_sensor(Sensor *s) { total_space_sensor_ = s; }
  void set_free_space_sensor(Sensor *s) { free_space_sensor_ = s; }
  void set_card_type_text_sensor(TextSensor *t) { card_type_text_ = t; }

  // Called by the framework every cycle
  void loop() override;

 protected:
  // internal
  std::string mount_point_;
  bool mode_1bit_;
  bool mounted_;
  void *card_handle_; // opaque pointer to sdmmc_card_t
  Sensor *used_space_sensor_{nullptr};
  Sensor *total_space_sensor_{nullptr};
  Sensor *free_space_sensor_{nullptr};
  TextSensor *card_type_text_{nullptr};
};

// Singleton accessor (convenience for lambdas)
SdCardBSP *get_sd_card_bsp();

}  // namespace sd_card_bsp
