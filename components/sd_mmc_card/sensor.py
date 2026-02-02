import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID
from . import SdMmcCard, sd_ns

CONF_TOTAL_SPACE = "total_space"
CONF_USED_SPACE = "used_space"
CONF_FREE_SPACE = "free_space"
CONF_FILE_SIZE = "file_size"
CONF_PATH = "path"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_ID): cv.use_id(SdMmcCard),
    cv.Optional(CONF_TOTAL_SPACE): sensor.sensor_schema(),
    cv.Optional(CONF_USED_SPACE): sensor.sensor_schema(),
    cv.Optional(CONF_FREE_SPACE): sensor.sensor_schema(),
    cv.Optional(CONF_FILE_SIZE): cv.Schema({
        cv.Required(CONF_PATH): cv.string,
    }).extend(sensor.sensor_schema()),
})

async def to_code(config):
    parent = await cg.get_variable(config[CONF_ID])
    
    if CONF_TOTAL_SPACE in config:
        sens = await sensor.new_sensor(config[CONF_TOTAL_SPACE])
        cg.add(parent.register_total_space_sensor(sens))
    
    if CONF_USED_SPACE in config:
        sens = await sensor.new_sensor(config[CONF_USED_SPACE])
        cg.add(parent.register_used_space_sensor(sens))
    
    if CONF_FREE_SPACE in config:
        sens = await sensor.new_sensor(config[CONF_FREE_SPACE])
        cg.add(parent.register_free_space_sensor(sens))
    
    if CONF_FILE_SIZE in config:
        sens = await sensor.new_sensor(config[CONF_FILE_SIZE])
        path = config[CONF_FILE_SIZE][CONF_PATH]
        cg.add(parent.register_file_size_sensor(sens, path))