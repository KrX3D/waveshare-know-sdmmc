import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from . import SdMmcCard
from esphome.const import CONF_ID, CONF_NAME

CONF_TYPE = "type"
TYPES = ["sd_card_type", "file_content"]

CONFIG_SCHEMA = cv.All(
    cv.Schema({
        cv.GenerateID(): cv.use_id(SdMmcCard),
        cv.Required(CONF_TYPE): cv.one_of(*TYPES),
        cv.Required(CONF_NAME): cv.string,
    })
)

async def to_code(config):
    sd = await cg.get_variable(config[cv.GenerateID()])
    ts = cg.new_Pvariable(config[cv.GenerateID()])

    if config[CONF_TYPE] == "sd_card_type":
        cg.add(text_sensor.register_text_sensor(ts, sd.card_type(), config[CONF_NAME]))
    elif config[CONF_TYPE] == "file_content":
        cg.add(text_sensor.register_text_sensor(ts, sd.file_content(), config[CONF_NAME]))

    return ts
