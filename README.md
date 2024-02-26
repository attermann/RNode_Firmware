# microReticulum_Firmware

Fork of RNode_Firmware with integration of the [microReticulum](https://github.com/attermann/microReticulum) Network Stack to implement a completeley self-contained standalone Reticulum node.

## Dependencies

Build environment is configured for use in [VSCode](https://code.visualstudio.com/) and [PlatformIO](https://platformio.org/).

## Building

Building and uploading to hardware is simple through the VSCode PlatformIO IDE
- Install VSCode and PlatformIO
- Clone this repo
- Lanch PlatformIO and load repo
- In PlatformIO, select the environment for intended board
- Build, Upload, and Monitor to observe application logging

Uploading to devices requires access to the `rnodeconf` utility included in the official [Reticulum](https://github.com/markqvist/Reticulum) distribution to update the device firmware hash. Without this step the device will report invalid firmware and will fail to fully initialize.

Instructions for command line builds and packaging for firmware distribution.

## Roadmap

- [ ] Extend KISS interface to support config/control of the integrated stack
- [ ] Add interface for easy customization of firmware
- [ ] Add power management and sleep states to extend battery runtime
- [ ] Add build targets for NRF52 boards

Please open an Issue if you have trouble building ior using the API, and feel free to start a new Discussion for anything else.

