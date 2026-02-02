import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from . import SdMmcCard, sd_ns

CONF_TYPE = "type"
TYPES = ["sd_card_type"]

CONFIG_SCHEMA = cv.All(
    cv.Schema({
        cv.GenerateID(): cv.use_id(SdMmcCard),
        cv.Required(CONF_TYPE): cv.one_of(*TYPES),
    })
)

async def to_code(config):
    var = await cg.get_variable(config[cv.GenerateID()])
    ts = cg.new_Pvariable(config[cv.GenerateID()])
    cg.add(var)  # ensure component is registered

    if config[CONF_TYPE] == "sd_card_type":
        cg.add(text_sensor.new_text_sensor(ts, var.card_type(), "SD Card Type"))
    return ts
