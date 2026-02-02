#pragma once
#include "esphome/core/component.h"

namespace esphome {
namespace sd_mmc_card {

class SdMmc : public Component {
 public:
  void set_clk_pin(int pin) { clk_pin_ = pin; }
  void set_cmd_pin(int pin) { cmd_pin_ = pin; }
  void set_data0_pin(int pin) { data0_pin_ = pin; }
  void set_data1_pin(int pin) { data1_pin_ = pin; }
  void set_data2_pin(int pin) { data2_pin_ = pin; }
  void set_data3_pin(int pin) { data3_pin_ = pin; }
  void set_mode_1bit(bool v) { mode_1bit_ = v; }
  void set_power_ctrl_pin(GPIOPin *pin) { power_ctrl_pin_ = pin; }

  void setup() override;

 protected:
  int clk_pin_;
  int cmd_pin_;
  int data0_pin_;
  int data1_pin_;
  int data2_pin_;
  int data3_pin_;
  bool mode_1bit_{false};
  GPIOPin *power_ctrl_pin_{nullptr};
};

}  // namespace sd_mmc_card
}  // namespace esphome
