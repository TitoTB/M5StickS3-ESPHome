import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import microphone
from esphome.const import CONF_ID, CONF_SAMPLE_RATE

DEPENDENCIES = ["esp32"]

m5sticks3_microphone_ns = cg.esphome_ns.namespace("m5sticks3_microphone")
M5StickS3Microphone = m5sticks3_microphone_ns.class_(
    "M5StickS3Microphone", microphone.Microphone, cg.Component
)

CONF_BUFFER_DURATION = "buffer_duration"

CONFIG_SCHEMA = microphone.MICROPHONE_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(M5StickS3Microphone),
        cv.Optional(CONF_SAMPLE_RATE, default=16000): cv.int_range(min=8000, max=48000),
        cv.Optional(CONF_BUFFER_DURATION, default="20ms"): cv.positive_time_period_milliseconds,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await microphone.register_microphone(var, config)

    cg.add(var.set_sample_rate(config[CONF_SAMPLE_RATE]))
    cg.add(var.set_buffer_duration(config[CONF_BUFFER_DURATION]))
