import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import binary_sensor, sensor, switch
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_VOLTAGE,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
    UNIT_AMPERE,
    UNIT_PERCENT,
    UNIT_VOLT,
)

DEPENDENCIES = ["esp32"]
AUTO_LOAD = ["binary_sensor", "sensor", "switch"]

m5sticks3_power_ns = cg.esphome_ns.namespace("m5sticks3_power")
M5StickS3Power = m5sticks3_power_ns.class_("M5StickS3Power", cg.PollingComponent)
M5StickS3Ext5VSwitch = m5sticks3_power_ns.class_(
    "M5StickS3Ext5VSwitch", switch.Switch, cg.Component
)

CONF_SDA = "sda"
CONF_SCL = "scl"
CONF_BATTERY_LEVEL = "battery_level"
CONF_BATTERY_VOLTAGE = "battery_voltage"
CONF_BATTERY_CURRENT = "battery_current"
CONF_CHARGING = "charging"
CONF_EXT_5V = "ext_5v"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(M5StickS3Power),
        cv.Optional(CONF_SDA, default="GPIO47"): pins.internal_gpio_output_pin_schema,
        cv.Optional(CONF_SCL, default="GPIO48"): pins.internal_gpio_output_pin_schema,
        cv.Optional(CONF_BATTERY_LEVEL): sensor.sensor_schema(
            unit_of_measurement=UNIT_PERCENT,
            icon="mdi:battery",
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_BATTERY,
            state_class=STATE_CLASS_MEASUREMENT,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_BATTERY_VOLTAGE): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_BATTERY_CURRENT): sensor.sensor_schema(
            unit_of_measurement=UNIT_AMPERE,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_CURRENT,
            state_class=STATE_CLASS_MEASUREMENT,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_CHARGING): binary_sensor.binary_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_EXT_5V): switch.switch_schema(
            M5StickS3Ext5VSwitch,
            icon="mdi:power-plug",
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
    }
).extend(cv.polling_component_schema("30s"))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    sda = await cg.gpio_pin_expression(config[CONF_SDA])
    scl = await cg.gpio_pin_expression(config[CONF_SCL])
    cg.add(var.set_i2c_pins(sda, scl))

    if battery_level_config := config.get(CONF_BATTERY_LEVEL):
        sens = await sensor.new_sensor(battery_level_config)
        cg.add(var.set_battery_level_sensor(sens))

    if battery_voltage_config := config.get(CONF_BATTERY_VOLTAGE):
        sens = await sensor.new_sensor(battery_voltage_config)
        cg.add(var.set_battery_voltage_sensor(sens))

    if battery_current_config := config.get(CONF_BATTERY_CURRENT):
        sens = await sensor.new_sensor(battery_current_config)
        cg.add(var.set_battery_current_sensor(sens))

    if charging_config := config.get(CONF_CHARGING):
        sens = await binary_sensor.new_binary_sensor(charging_config)
        cg.add(var.set_charging_binary_sensor(sens))

    if ext_5v_config := config.get(CONF_EXT_5V):
        sw = await switch.new_switch(ext_5v_config)
        await cg.register_component(sw, ext_5v_config)
        cg.add(sw.set_parent(var))
        cg.add(var.set_ext_5v_switch(sw))
