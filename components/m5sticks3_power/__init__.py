import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, button, sensor, switch
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_VOLTAGE,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
    UNIT_PERCENT,
    UNIT_VOLT,
)

DEPENDENCIES = ["esp32"]
AUTO_LOAD = ["binary_sensor", "button", "sensor", "switch"]

m5sticks3_power_ns = cg.esphome_ns.namespace("m5sticks3_power")
M5StickS3Power = m5sticks3_power_ns.class_("M5StickS3Power", cg.PollingComponent)
M5StickS3Ext5VSwitch = m5sticks3_power_ns.class_(
    "M5StickS3Ext5VSwitch", switch.Switch, cg.Component
)
M5StickS3BeepButton = m5sticks3_power_ns.class_(
    "M5StickS3BeepButton", button.Button, cg.Component
)

CONF_BATTERY_LEVEL = "battery_level"
CONF_BATTERY_VOLTAGE = "battery_voltage"
CONF_INPUT_VOLTAGE = "input_voltage"
CONF_FIVE_VOLT_VOLTAGE = "five_volt_voltage"
CONF_CHARGING = "charging"
CONF_EXT_5V = "ext_5v"
CONF_BEEP = "beep"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(M5StickS3Power),
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
        cv.Optional(CONF_INPUT_VOLTAGE): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_FIVE_VOLT_VOLTAGE): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
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
        cv.Optional(CONF_BEEP): button.button_schema(
            M5StickS3BeepButton,
            icon="mdi:volume-high",
        ),
    }
).extend(cv.polling_component_schema("30s"))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    if battery_level_config := config.get(CONF_BATTERY_LEVEL):
        sens = await sensor.new_sensor(battery_level_config)
        cg.add(var.set_battery_level_sensor(sens))

    if battery_voltage_config := config.get(CONF_BATTERY_VOLTAGE):
        sens = await sensor.new_sensor(battery_voltage_config)
        cg.add(var.set_battery_voltage_sensor(sens))

    if input_voltage_config := config.get(CONF_INPUT_VOLTAGE):
        sens = await sensor.new_sensor(input_voltage_config)
        cg.add(var.set_input_voltage_sensor(sens))

    if five_volt_voltage_config := config.get(CONF_FIVE_VOLT_VOLTAGE):
        sens = await sensor.new_sensor(five_volt_voltage_config)
        cg.add(var.set_five_volt_voltage_sensor(sens))

    if charging_config := config.get(CONF_CHARGING):
        sens = await binary_sensor.new_binary_sensor(charging_config)
        cg.add(var.set_charging_binary_sensor(sens))

    if ext_5v_config := config.get(CONF_EXT_5V):
        sw = await switch.new_switch(ext_5v_config)
        await cg.register_component(sw, ext_5v_config)
        cg.add(sw.set_parent(var))
        cg.add(var.set_ext_5v_switch(sw))

    if beep_config := config.get(CONF_BEEP):
        btn = await button.new_button(beep_config)
        await cg.register_component(btn, beep_config)
        cg.add(btn.set_parent(var))
