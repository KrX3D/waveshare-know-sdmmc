import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from . import SdMmcCard, sd_ns
from esphome.const import CONF_NAME

CONF_TYPE = "type"
TYPES = ["sd_card_type"]

CONFIG_SCHEMA = cv.All(
    cv.Schema({
        cv.GenerateID(): cv.use_id(SdMmcCard),
        cv.Required(CONF_TYPE): cv.one_of(*TYPES),
        cv.Required(CONF_NAME): cv.string,  # <-- add name here
    })
)

async def to_code(config):
    var = await cg.get_variable(config[cv.GenerateID()])
    ts = cg.new_Pvariable(config[cv.GenerateID()])
    cg.add(var)  # ensure the component is registered

    cg.add(text_sensor.register_text_sensor(ts, var.card_type(), config[CONF_NAME]))
    return ts
