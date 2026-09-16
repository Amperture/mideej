# mideej
A class-compliant USB MIDI fader box, built on [Zephyr RTOS](https://zephyrproject.org/) and an ESP32-S3.
## Status

Early.

Working:

- Enumerates as a class-compliant USB MIDI 2.0 device
- Sends MIDI control-change messages that arrive on a Linux host with no
  host-side driver or configuration

Not yet:

- More channels. The firmware currently drives a single control change.
- External ADC. An 8-channel I2C ADC (TI ADS7830) is the current intended input,
  to be implemented later.
- Hardware. There is no PCB yet; this runs on an ESP32-S3 DevKitC.


## Why MIDI instead of a serial protocol

This project is heavily inspired by [deej](https://github.com/omriharel/deej), which solves the same problem and
solved it first.

However, using a serial port implementation seems like a less appropriate
means to accomplish the same effect.

- **No companion software to install or keep running.** Anything MIDI-aware
  consumes it directly. Examples include OBS via a MIDI plugin, a DAW, etc.
- **No custom protocol to specify or maintain.** MIDI CC is a defined wire
  format with decades of tooling behind it.
- **No serial port to find.** No `/dev/ttyUSB*` enumeration order, no `dialout`
  group, no COM port number that changes when you use a different socket.
- **Cross-platform for free.** Class-compliant means every major OS binds it
  without a driver, because the descriptors say what it is.

## Hardware
Bear in mind that hardware is considered both early, and subject to change or
configuration.

- **ESP32-S3-DevKitC**
- **TI ADS7830**: 8-channel, 8-bit I2C ADC
- Eight slide potentiometers
## Building

Currently requires a Zephyr development environment and `west`.

```sh
west build -p always -b esp32s3_devkitc/esp32s3/procpu .
west flash
```

## Acknowledgements

[deej](https://github.com/omriharel/deej) by Omri Harel: the original, and the project that made the idea obvious.

## Licence

Apache-2.0. See [LICENSE](LICENSE).
