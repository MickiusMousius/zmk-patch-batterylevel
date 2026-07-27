/*
 * Copyright (c) 2021 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_battery_voltage_divider_curve

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include "battery_curve_common.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define BATT_SAMPLES (3)

struct io_channel_config
{
    uint8_t channel;
};

struct bvdc_config
{
    struct io_channel_config io_channel;
    uint32_t output_ohm;
    uint32_t full_ohm;
};

struct bvdc_data
{
    const struct device *adc;
    struct adc_channel_cfg acc;
    struct adc_sequence as;
    struct battery_curve_value value;
};

static int bvdc_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
    struct bvdc_data *drv_data = dev->data;
    const struct bvdc_config *drv_cfg = dev->config;
    struct adc_sequence *as = &drv_data->as;

    if (chan != SENSOR_CHAN_GAUGE_VOLTAGE && chan != SENSOR_CHAN_GAUGE_STATE_OF_CHARGE &&
        chan != SENSOR_CHAN_ALL)
    {
        LOG_DBG("Selected channel is not supported: %d.", chan);
        return -ENOTSUP;
    }

    int32_t mv[BATT_SAMPLES];

    for (int i = 0; i < BATT_SAMPLES; i++)
    {
        int rc = adc_read(drv_data->adc, as);
        as->calibrate = false;

        if (rc != 0)
        {
            LOG_ERR("Failed to read ADC: %d", rc);
            return rc;
        }

        int32_t val = drv_data->value.adc_raw;
        adc_raw_to_millivolts(adc_ref_internal(drv_data->adc), drv_data->acc.gain, as->resolution, &val);

        // Cast to uint64_t to prevent overflow before division
        mv[i] = val * (uint64_t)drv_cfg->full_ohm / drv_cfg->output_ohm;
    }

    const int32_t median = MAX(MIN(mv[0], mv[1]), MIN(MAX(mv[0], mv[1]), mv[2]));

    drv_data->value.millivolts = median;
    drv_data->value.state_of_charge = lipo_mv_to_pct(median);

    LOG_DBG("ADC median %d mV => %d%%", median, drv_data->value.state_of_charge);

    return 0;
}

static int bvdc_channel_get(const struct device *dev, enum sensor_channel chan,
                            struct sensor_value *val)
{
    struct bvdc_data *drv_data = dev->data;
    return battery_curve_channel_get(&drv_data->value, chan, val);
}

static const struct sensor_driver_api bvdc_api = {
    .sample_fetch = bvdc_sample_fetch,
    .channel_get = bvdc_channel_get,
};

static int bvdc_init(const struct device *dev)
{
    struct bvdc_data *drv_data = dev->data;
    const struct bvdc_config *drv_cfg = dev->config;

    if (drv_data->adc == NULL)
    {
        LOG_ERR("ADC failed to retrieve ADC driver");
        return -ENODEV;
    }

    drv_data->as = (struct adc_sequence){
        .channels = BIT(0),
        .buffer = &drv_data->value.adc_raw,
        .buffer_size = sizeof(drv_data->value.adc_raw),
        .oversampling = 4,
        .calibrate = true,
    };

#ifdef CONFIG_ADC_NRFX_SAADC
    drv_data->acc = (struct adc_channel_cfg){
        .gain = ADC_GAIN_1_6,
        .reference = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 40),
        .input_positive = SAADC_CH_PSELP_PSELP_AnalogInput0 + drv_cfg->io_channel.channel,
    };

    drv_data->as.resolution = 12;
#else
#error Unsupported ADC
#endif

    const int rc = adc_channel_setup(drv_data->adc, &drv_data->acc);
    LOG_DBG("AIN%u setup returned %d", drv_cfg->io_channel.channel, rc);

    return rc;
}

#define BVDC_INIT(n)                                                                       \
    static struct bvdc_data bvdc_data_##n = {                                              \
        .adc = DEVICE_DT_GET(DT_IO_CHANNELS_CTLR(DT_DRV_INST(n)))};                        \
    static const struct bvdc_config bvdc_cfg_##n = {                                       \
        .io_channel = {                                                                    \
            DT_IO_CHANNELS_INPUT(DT_DRV_INST(n)),                                          \
        },                                                                                 \
        .output_ohm = DT_INST_PROP(n, output_ohms),                                        \
        .full_ohm = DT_INST_PROP(n, full_ohms),                                            \
    };                                                                                     \
    DEVICE_DT_INST_DEFINE(n, &bvdc_init, NULL, &bvdc_data_##n, &bvdc_cfg_##n, POST_KERNEL, \
                          CONFIG_SENSOR_INIT_PRIORITY, &bvdc_api);

DT_INST_FOREACH_STATUS_OKAY(BVDC_INIT)