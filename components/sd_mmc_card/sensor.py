import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from . import SdMmcCard, sd_ns
from esphome.const import CONF_ID, CONF_NAME, CONF_PATH, CONF_TYPE

CONF_SENSOR_TYPE = "type"
SENSOR_TYPES = ["total_space", "used_space", "free_space", "file_size"]

# Schema for SD card sensors
CONFIG_SCHEMA = cv.All(
    cv.Schema({
        cv.GenerateID(): cv.use_id(SdMmcCard),
        cv.Required(CONF_SENSOR_TYPE): cv.one_of(*SENSOR_TYPES),
        cv.Required(CONF_NAME): cv.string,
        cv.Optional(CONF_PATH): cv.string,  # required only for file_size
    })
)

async def to_code(config):
    sd = await cg.get_variable(config[cv.GenerateID()])
    var = cg.new_Pvariable(config[cv.GenerateID()])  # register component if not done
    cg.add(sd)  # ensure component is registered

    if config[CONF_SENSOR_TYPE] == "total_space":
        s = sensor.new_sensor(config[CONF_NAME])
        cg.add(sd.register_total_space_sensor(s))
        return s
    elif config[CONF_SENSOR_TYPE] == "used_space":
        s = sensor.new_sensor(config[CONF_NAME])
        cg.add(sd.register_used_space_sensor(s))
        return s
    elif config[CONF_SENSOR_TYPE] == "free_space":
        s = sensor.new_sensor(config[CONF_NAME])
        cg.add(sd.register_free_space_sensor(s))
        return s
    elif config[CONF_SENSOR_TYPE] == "file_size":
        path = config[CONF_PATH]
        s = sensor.new_sensor(config[CONF_NAME])
        cg.add(sd.register_file_size_sensor(s, path))
        return s
