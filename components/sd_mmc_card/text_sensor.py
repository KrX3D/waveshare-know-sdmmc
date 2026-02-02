import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ID, CONF_TYPE
from . import SdMmcCard, sd_ns

CONF_SD_MMC_CARD_ID = "sd_mmc_card_id"

TEXT_SENSOR_TYPES = {
    "sd_card_type": "SD Card Type",
    "file_content": "File Content",
    "fs_type": "Filesystem Type",
}

CONFIG_SCHEMA = text_sensor.text_sensor_schema().extend({
    cv.Optional(CONF_SD_MMC_CARD_ID): cv.use_id(SdMmcCard),
    cv.Required(CONF_TYPE): cv.enum(TEXT_SENSOR_TYPES, lower=True),
})

async def to_code(config):
    sens = await text_sensor.new_text_sensor(config)
    
    sensor_type = config[CONF_TYPE]
    parent = None
    if CONF_SD_MMC_CARD_ID in config:
        parent = await cg.get_variable(config[CONF_SD_MMC_CARD_ID])

    parent_expr = "sd_mmc_card::SdMmcCard::get_default()"

    def add_default(expression):
        cg.add(
            cg.RawExpression(
                f"if (auto *sd = {parent_expr}) {{ {expression} }} else {{ ESP_LOGE(\"sd_mmc_card\", \"Missing sd_mmc_card_id and no default instance\"); }}"
            )
        )
    
    if sensor_type == "sd_card_type":
        if parent is None:
            add_default(f"sd->register_card_type_text_sensor({sens})")
        else:
            cg.add(parent.register_card_type_text_sensor(sens))
    elif sensor_type == "file_content":
        if parent is None:
            add_default(f"sd->register_file_content_text_sensor({sens})")
        else:
            cg.add(parent.register_file_content_text_sensor(sens))
    elif sensor_type == "fs_type":
        if parent is None:
            add_default(f"sd->register_fs_type_text_sensor({sens})")
        else:
            cg.add(parent.register_fs_type_text_sensor(sens))
