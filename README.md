# OpenRUNN

OpenRUNN is an ESP32-C3 treadmill running sensor. It reads pulses from a TCRT5000 optical sensor and exposes speed and distance through the Bluetooth Low Energy Running Speed and Cadence (RSC) service.

## Requirements

- Python 3
- A USB connection to an ESP32-C3 DevKitM-1
- macOS, Linux, or Windows

The first build downloads the ESP32 platform, Arduino framework, and other PlatformIO packages automatically.

## Python Virtual Environment

From the project root, create and activate a virtual environment.

### macOS and Linux

```sh
python3 -m venv .venv
source .venv/bin/activate
python -m pip install --upgrade pip
python -m pip install -r requirements.txt
```

### Windows PowerShell

```powershell
py -3 -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
python -m pip install -r requirements.txt
```

The virtual environment must be activated before using the `pio` commands below. To leave it, run:

```sh
deactivate
```

## Configure

Measure the distance represented by one valid sensor pulse, in meters, and update `main.cpp`:

```cpp
const float beltLengthMeters = 3.19f;        // Distance per valid sensor pulse
```

After changing this value, build and upload the firmware again.

## Build

Compile the firmware for the ESP32-C3 DevKitM-1:

```sh
pio run --environment esp32-c3-devkitm-1
```

The first build may take longer while PlatformIO downloads the required toolchains and frameworks.

## Upload

Connect the board by USB and upload the firmware:

```sh
pio run --target upload --environment esp32-c3-devkitm-1
```

If PlatformIO does not detect the board automatically, list available serial ports:

```sh
pio device list
```

Then specify the port explicitly:

```sh
pio run --target upload --environment esp32-c3-devkitm-1 --upload-port /dev/cu.usbmodemXXXX
```

Replace `/dev/cu.usbmodemXXXX` with the port reported on your system.

## Serial Monitor

Open the serial monitor at the configured baud rate:

```sh
pio device monitor --environment esp32-c3-devkitm-1 --baud 115200
```

To specify the port explicitly:

```sh
pio device monitor --environment esp32-c3-devkitm-1 --port /dev/cu.usbmodemXXXX --baud 115200
```

The firmware prints startup, Bluetooth connection, disconnection, and RSC measurement messages to the monitor.

## Useful Commands

Clean the build directory:

```sh
pio run --target clean --environment esp32-c3-devkitm-1
```

Build and upload in one command:

```sh
pio run --target upload --environment esp32-c3-devkitm-1
```

## Project Structure

```text
.
|-- platformio.ini       # PlatformIO project configuration
|-- requirements.txt     # Python dependency for PlatformIO Core
`-- src/
    |-- main.cpp         # Application setup and main loop
    |-- BLE_RSC.cpp/.h   # Bluetooth Low Energy RSC service
    `-- TCRT5000.cpp/.h  # Optical sensor and pulse measurement
```
