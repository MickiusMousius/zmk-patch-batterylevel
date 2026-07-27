/*
 * Copyright (c) 2021 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zephyr/drivers/sensor.h>
#include <stdint.h>

struct battery_curve_value
{
    uint16_t adc_raw;
    uint16_t millivolts;
    uint8_t state_of_charge;
};

int battery_curve_channel_get(const struct battery_curve_value *value, enum sensor_channel chan,
                              struct sensor_value *val_out);

uint8_t lipo_mv_to_pct(int32_t mv);