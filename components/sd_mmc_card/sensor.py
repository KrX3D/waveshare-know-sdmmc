import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from . import SdMmcCard, sd_ns

CONF_TYPE = "type"
CONF_PATH = "path"

TYPES = ["total_space", "free_space", "used_space", "file_size"]

sd_sensor_ns = sd_ns.namespace("sensor")

CONFIG_SCHEMA = cv.All(
    cv.Schema({
        cv.GenerateID(): cv.use_id(SdMmcCard),
        cv.Required(CONF_TYPE): cv.one_of(*TYPES),
        cv.Optional(CONF_PATH): cv.string,  # only for file_size
    }),
    cv.has_at_least_one_key(CONF_TYPE)
)

async def to_code(config):
    var = await cg.get_variable(config[cv.GenerateID()])
    sens = cg.new_Pvariable(config[cv.GenerateID()])
    cg.add(var)  # ensure the component is registered

    if config[CONF_TYPE] == "total_space":
        cg.add(sensor.new_sensor(sens, var.total_space(), "Total SD space", "B"))
    elif config[CONF_TYPE] == "free_space":
        cg.add(sensor.new_sensor(sens, var.free_space(), "Free SD space", "B"))
    elif config[CONF_TYPE] == "used_space":
        cg.add(sensor.new_sensor(sens, var.used_space(), "Used SD space", "B"))
    elif config[CONF_TYPE] == "file_size":
        path = config[CONF_PATH]
        cg.add(sensor.new_sensor(sens, var.file_size(path), f"{path} size", "B"))
    return sens
