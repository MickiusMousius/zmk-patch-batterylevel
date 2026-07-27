zephyr_library()

# Add the shared utility code
zephyr_library_sources(src/sensors/battery_curve_common.c)

# Add the specific drivers conditionally
zephyr_library_sources_ifdef(CONFIG_ZMK_BATTERY_NRF_VDDH_CURVE src/sensors/battery_nrf_vddh_curve.c)
zephyr_library_sources_ifdef(CONFIG_ZMK_BATTERY_VOLTAGE_DIVIDER_CURVE src/sensors/battery_voltage_divider_curve.c)