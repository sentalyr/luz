# luz

ESP32 based LED controller for standard rock climbing boards.

The luz application advertises a BLE service compatible with the Aurora Climbing mobile apps (e.g. Decoy, Kilter, Grasshopper, etc).
Once paired with the app, the controller illuminates the holds corresponding to the selected climb.

The project currently implements the 12x12 Decoy board interface.
Support for other boards can be implemented by creating a lookup table mapping the hold position to the pixel index (see the Decoy board mapping in [luz/database.cc](luz/database.cc).

# Installation

* Clone this repository

## Prerequisites

* [esp-idf](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html#installation)
* [Optional] Install [direnv](https://direnv.net/#getting-started)
*   Used to autoload esp-idf and configure pre-commit hooks

## Build

* `cd` to project directory
* Run `luz build` to build the project
* Run `luz flash` to flash a connected ESP32

# Additional Resources

* [BoM](docs/bom.md) - sample hardware Bill of Materials
* [Wiring Diagram](docs/wiring.md) - wiring "schematics"

