idf_component_register(
    SRCS "app_main.c" "thermal.c" "tmp117.c"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_i2c esp_timer freertos
)
