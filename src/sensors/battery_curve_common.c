/*
 * Copyright (c) 2021 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 */

#include "battery_curve_common.h"
#include <zephyr/sys/util.h>

int battery_curve_channel_get(const struct battery_curve_value *value, enum sensor_channel chan,
                              struct sensor_value *val_out)
{
    switch (chan)
    {
    case SENSOR_CHAN_GAUGE_VOLTAGE:
        val_out->val1 = value->millivolts / 1000;
        val_out->val2 = (value->millivolts % 1000) * 1000U;
        break;
    case SENSOR_CHAN_GAUGE_STATE_OF_CHARGE:
        val_out->val1 = value->state_of_charge;
        val_out->val2 = 0;
        break;
    default:
        return -ENOTSUP;
    }
    return 0;
}

struct curve_point
{
    int16_t millivolts;
    int8_t percent;
};

// 1S LiPo rest-discharge anchors
static const struct curve_point lipo_curve[] = {
    {4200, 100},
    {4150, 95},
    {4110, 90},
    {4080, 85},
    {4020, 80},
    {3980, 75},
    {3950, 70},
    {3910, 65},
    {3870, 60},
    {3850, 55},
    {3840, 50},
    {3820, 45},
    {3800, 40},
    {3790, 35},
    {3770, 30},
    {3750, 25},
    {3730, 20},
    {3710, 15},
    {3690, 10},
    {3610, 5},
    {3270, 0},
};

uint8_t lipo_mv_to_pct(int32_t mv)
{
    if (mv >= lipo_curve[0].millivolts)
    {
        return lipo_curve[0].percent;
    }
    for (size_t i = 1; i < ARRAY_SIZE(lipo_curve); i++)
    {
        const struct curve_point *upper = &lipo_curve[i - 1];
        const struct curve_point *lower = &lipo_curve[i];
        if (mv >= lower->millivolts)
        {
            return lower->percent + (int32_t)(upper->percent - lower->percent) *
                                        (mv - lower->millivolts) /
                                        (upper->millivolts - lower->millivolts);
        }
    }
    return 0;
}