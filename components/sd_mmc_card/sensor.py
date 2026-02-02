import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from . import SdMmcCard
from esphome.const import CONF_ID, CONF_NAME, CONF_PATH, CONF_TYPE

CONF_SENSOR_TYPE = "type"
SENSOR_TYPES = ["total_space", "used_space", "free_space", "file_size"]

CONFIG_SCHEMA = cv.All(
    cv.Schema({
        cv.GenerateID(): cv.use_id(SdMmcCard),
        cv.Required(CONF_SENSOR_TYPE): cv.one_of(*SENSOR_TYPES),
        cv.Required(CONF_NAME): cv.string,
        cv.Optional(CONF_PATH): cv.string,  # required for file_size
    })
)

async def to_code(config):
    sd = await cg.get_variable(config[cv.GenerateID()])

    s = sensor.new_sensor(config[CONF_NAME])

    if config[CONF_SENSOR_TYPE] == "total_space":
        cg.add(sd.register_total_space_sensor(s))
    elif config[CONF_SENSOR_TYPE] == "used_space":
        cg.add(sd.register_used_space_sensor(s))
    elif config[CONF_SENSOR_TYPE] == "free_space":
        cg.add(sd.register_free_space_sensor(s))
    elif config[CONF_SENSOR_TYPE] == "file_size":
        path = config[CONF_PATH]
        cg.add(sd.register_file_size_sensor(s, path))

    return s
