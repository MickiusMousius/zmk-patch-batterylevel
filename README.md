# ZMK Battery Level Curve Module

This ZMK module provides drop-in replacements for ZMK's default battery drivers. It is designed to solve two common issues with custom wireless mechanical keyboards: wildly inaccurate battery percentages and sudden percentage drops (jitter) caused by voltage sag during BLE transmissions or display updates.

# Why Use This Over ZMK's Default?

ZMK's default battery driver calculates your remaining battery using a single straight-line estimate. However, Lithium Polymer (LiPo) batteries do not discharge linearly; they hold their voltage for a while, drop steadily, and then plummet abruptly at the end. Because of this, a straight-line calculation often overstates the low end of the battery scale by 20 or more percentage points.

By using a 21-point interpolation curve based on actual 1S LiPo rest-discharge data, this module provides a significantly more accurate representation of your true remaining battery life.

Furthermore, the default driver can suffer from percentage "jitter" caused by momentary voltage sags when the Bluetooth radio transmits or an e-ink screen refreshes. This module fixes that by rapidly sampling the ADC three times and taking the median value, filtering out those temporary sags to provide a stable, consistent reading.

# Features

- **Piecewise-Linear 1S LiPo Curve:** Converts millivolts to a percentage using a 21-point interpolation curve based on standard 1S LiPo rest-discharge data.
- **Median-of-Three Sampling:** Rapidly samples the ADC three times and uses the median value to filter out transient voltage sags.
- **Dual Hardware Support:** Includes drivers for both direct nRF VDDH reading and standard voltage dividers.
- **Chip-Agnostic:** Built using standard Zephyr Devicetree API macros, making the voltage divider driver fully compatible with any microcontroller ZMK supports.
- **Namespace Safe:** Uses isolated functions (e.g., `battery_curve_channel_get`) to cleanly compile alongside standard ZMK firmware without linker collisions.

## Drivers Included

1. `zmk,battery-nrf-vddh-curve`: A replacement for `zmk,battery-nrf-vddh`.
2. `zmk,battery-voltage-divider-curve`: A replacement for `zmk,battery-voltage-divider`.

# Installation

To use this module, you need to add it to your ZMK user config repository.

Open your `config/west.yml` file and add this repository to your `remotes` and `projects` sections:

```yaml
manifest:
  remotes:
    - name: zmkfirmware
      url-base: [https://github.com/zmkfirmware](https://github.com/zmkfirmware)
    # GitHub username & organization here
    - name: MickiusMousius
      url-base: https://github.com/MickiusMousius

  projects:
    - name: zmk
      remote: zmkfirmware
      revision: main
      import: app/west.yml
    # Module here
    - name: zmk-patch-batterylevel
      remote: MickiusMousius
      revision: main

  self:
    path: config

```

## Configuration

To use the drivers, you will need to update your board or shield's `.dts` or `.overlay` file to use the new `compatible` strings. You also need to ensure the `zmk,battery` chosen node points to your configured sensor.

### Option A: nRF VDDH Reading

If your board reads the battery directly through the nRF52's internal VDDH pin, use this configuration:

```dts
/ {
    chosen {
        zmk,battery = &vbatt;
    };

    vbatt: vbatt {
        compatible = "zmk,battery-nrf-vddh-curve";
    };
};

```

### Option B: Voltage Divider

If your board uses a physical resistor divider to read the battery voltage through a standard ADC pin, use this configuration. Make sure to adjust the `io-channels` and resistor values to match your specific hardware:

```dts
/ {
    chosen {
        zmk,battery = &vbatt;
    };

    vbatt: vbatt {
        compatible = "zmk,battery-voltage-divider-curve";
        io-channels = <&adc 6>;
        output-ohms = <1920000>;
        full-ohms = <(1920000 + 806000)>;
    };
};

```

# Credits & Attribution

The battery monitoring source code in this module was originally adapted and expanded from the configurations found in [reybits/zmk-config](https://github.com/reybits/zmk-config).

# License

This project is licensed under the MIT License.
