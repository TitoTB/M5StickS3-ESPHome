import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import display
from esphome.const import CONF_ID, CONF_LAMBDA

DEPENDENCIES = ["esp32"]
AUTO_LOAD = ["display"]

m5sticks3_display_ns = cg.esphome_ns.namespace("m5sticks3_display")
M5StickS3Display = m5sticks3_display_ns.class_(
    "M5StickS3Display", cg.PollingComponent, display.DisplayBuffer
)

CONF_SDA = "sda"
CONF_SCL = "scl"

CONFIG_SCHEMA = display.FULL_DISPLAY_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(M5StickS3Display),
        cv.Optional(CONF_SDA, default=47): cv.int_range(min=0, max=48),
        cv.Optional(CONF_SCL, default=48): cv.int_range(min=0, max=48),
    }
).extend(cv.polling_component_schema("1s"))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await display.register_display(var, config)

    cg.add(var.set_i2c_pins(config[CONF_SDA], config[CONF_SCL]))

    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA],
            [(display.DisplayRef, "it")],
            return_type=cg.void,
        )
        cg.add(var.set_writer(lambda_))
