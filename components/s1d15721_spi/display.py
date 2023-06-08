import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import display, spi
from esphome.const import (
    CONF_DC_PIN,
    CONF_ID,
    CONF_LAMBDA,
    CONF_MODEL,
    CONF_RESET_PIN,
    #CONF_WIDTH,
    #CONF_HEIGHT,
    CONF_INVERT,
)
s1d15721_ns = cg.esphome_ns.namespace("s1d15721_spi")

DEPENDENCIES = ["spi"]

s1d15721 = s1d15721_ns.class_(
    "SPIs1d15721", cg.PollingComponent, spi.SPIDevice, display.DisplayBuffer
)
s1d15721Ref = s1d15721.operator("ref")
s1d15721Model = s1d15721_ns.enum("s1d15721Model")

MODELS = {
    "S1D15721_240X64": s1d15721Model.s1d15721_MODEL_240_64,
}

s1d15721_MODEL = cv.enum(MODELS, upper=True, space="_")

CONFIG_SCHEMA = cv.All(
    display.FULL_DISPLAY_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(s1d15721),
            cv.Required(CONF_MODEL): s1d15721_MODEL,
            cv.Optional(CONF_RESET_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_DC_PIN): pins.gpio_output_pin_schema,
            #cv.Optional(CONF_HEIGHT): cv.int_,
            #cv.Optional(CONF_WIDTH): cv.int_,
            cv.Optional(CONF_INVERT, default=False): cv.boolean
        }
    )
    .extend(cv.polling_component_schema("5s"))
    .extend(spi.spi_device_schema(cs_pin_required=False)),
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await display.register_display(var, config)
    await spi.register_spi_device(var, config)

    cg.add(var.set_model(config[CONF_MODEL]))

    dc = await cg.gpio_pin_expression(config[CONF_DC_PIN])
    cg.add(var.set_dc_pin(dc))

    if CONF_RESET_PIN in config:
        reset = await cg.gpio_pin_expression(config[CONF_RESET_PIN])
        cg.add(var.set_reset_pin(reset))
    if CONF_INVERT in config:
        cg.add(var.init_invert(config[CONF_INVERT]))
    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA], [(display.DisplayBufferRef, "it")], return_type=cg.void
        )
        cg.add(var.set_writer(lambda_))
