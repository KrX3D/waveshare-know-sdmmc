import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

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
    cv.Required(CONF_CLK_PIN): cv.uint8_t,
    cv.Required(CONF_CMD_PIN): cv.uint8_t,
    cv.Required(CONF_DATA0_PIN): cv.uint8_t,
    cv.Optional(CONF_DATA1_PIN): cv.uint8_t,
    cv.Optional(CONF_DATA2_PIN): cv.uint8_t,
    cv.Optional(CONF_DATA3_PIN): cv.uint8_t,
    cv.Optional(CONF_MODE_1BIT, default=False): cv.boolean,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(cfg):
    var = cg.new_Pvariable(cfg[CONF_ID])
    await cg.register_component(var, cfg)

    cg.add(var.set_clk_pin(cfg[CONF_CLK_PIN]))
    cg.add(var.set_cmd_pin(cfg[CONF_CMD_PIN]))
    cg.add(var.set_data0_pin(cfg[CONF_DATA0_PIN]))
    cg.add(var.set_mode_1bit(cfg[CONF_MODE_1BIT]))

    if not cfg[CONF_MODE_1BIT]:
        cg.add(var.set_data1_pin(cfg[CONF_DATA1_PIN]))
        cg.add(var.set_data2_pin(cfg[CONF_DATA2_PIN]))
        cg.add(var.set_data3_pin(cfg[CONF_DATA3_PIN]))
