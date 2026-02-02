import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome import pins

DEPENDENCIES = ["esp32"]

sd_ns = cg.esphome_ns.namespace("sd_mmc_card")
SdMmcCard = sd_ns.class_("SdMmcCard", cg.Component)

CONF_CLK_PIN = "clk_pin"
CONF_CMD_PIN = "cmd_pin"
CONF_DATA0_PIN = "data0_pin"
CONF_DATA1_PIN = "data1_pin"
CONF_DATA2_PIN = "data2_pin"
CONF_DATA3_PIN = "data3_pin"
CONF_MODE_1BIT = "mode_1bit"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(SdMmcCard),
    cv.Required("clk_pin"): pins.internal_gpio_output_pin_number,
    cv.Required("cmd_pin"): pins.internal_gpio_output_pin_number,
    cv.Required("data0_pin"): pins.internal_gpio_pin_number,
    cv.Optional("data1_pin"): pins.internal_gpio_pin_number,
    cv.Optional("data2_pin"): pins.internal_gpio_pin_number,
    cv.Optional("data3_pin"): pins.internal_gpio_pin_number,
    cv.Optional("mode_1bit", default=False): cv.boolean,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(cfg):
    var = cg.new_Pvariable(cfg[CONF_ID])
    # Prevent registering the same component twice
    if not hasattr(var, "_registered"):
        await cg.register_component(var, cfg)
        var._registered = True

    cg.add(var.set_clk_pin(cfg[CONF_CLK_PIN]))
    cg.add(var.set_cmd_pin(cfg[CONF_CMD_PIN]))
    cg.add(var.set_data0_pin(cfg[CONF_DATA0_PIN]))
    cg.add(var.set_mode_1bit(cfg[CONF_MODE_1BIT]))

    if not cfg[CONF_MODE_1BIT]:
        cg.add(var.set_data1_pin(cfg[CONF_DATA1_PIN]))
        cg.add(var.set_data2_pin(cfg[CONF_DATA2_PIN]))
        cg.add(var.set_data3_pin(cfg[CONF_DATA3_PIN]))
