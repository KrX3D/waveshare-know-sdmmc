import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ID
from . import SdMmcCard, sd_ns

CONF_SD_CARD_TYPE = "sd_card_type"
CONF_FILE_CONTENT = "file_content"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_ID): cv.use_id(SdMmcCard),
    cv.Optional(CONF_SD_CARD_TYPE): text_sensor.text_sensor_schema(),
    cv.Optional(CONF_FILE_CONTENT): text_sensor.text_sensor_schema(),
})

async def to_code(config):
    parent = await cg.get_variable(config[CONF_ID])
    
    if CONF_SD_CARD_TYPE in config:
        sens = await text_sensor.new_text_sensor(config[CONF_SD_CARD_TYPE])
        cg.add(parent.register_card_type_text_sensor(sens))
    
    if CONF_FILE_CONTENT in config:
        sens = await text_sensor.new_text_sensor(config[CONF_FILE_CONTENT])
        cg.add(parent.register_file_content_text_sensor(sens))