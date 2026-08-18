/*
 * Copyright (c) 2021 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 * @file battery_curve_common.h
 * @brief Common definitions and algorithms for translating battery voltage into state of charge.
 * @details Defines the core data structures and lookup logic shared between different battery sensor implementations.
 * It provides a generalized curve mapping millivolts to percentage to keep battery reporting consistent across varied
 * hardware designs.
 */

/* ========================================================================= */
/*                        INCLUDES AND DEPENDENCIES                          */
/* ========================================================================= */
#pragma once

#include <stdint.h>
#include <zephyr/drivers/sensor.h>

/* ========================================================================= */
/*                            DATA STRUCTURES                                */
/* ========================================================================= */
struct battery_curve_value {
    uint16_t adc_raw;
    uint16_t millivolts;
    uint8_t state_of_charge;
};


/* ========================================================================= */
/*                               PUBLIC API                                  */
/* ========================================================================= */
int battery_curve_channel_get(const struct battery_curve_value *value, enum sensor_channel chan,
                              struct sensor_value *val_out);

uint8_t lipo_mv_to_pct(int32_t mv);