# ESP32 RF Lab

ESP32 RF Lab is a portable Wi-Fi and RF experimentation and benchmarking toolkit for the ESP32 family. It is designed to evaluate antenna performance, signal strength, and connection quality without relying on external UI components like OLEDs or buttons.

## Features

- **Wi-Fi Network Discovery**: Scan for networks and display signal strength, channel, and BSSID.
- **Hardware Agnostic**: Supports ESP32, ESP32-S2, ESP32-S3, ESP32-C3, and ESP32-C6.
- **Signal-Strength Analysis**: Real-time RSSI measurement and statistical calculations (min, max, avg, variance).
- **Antenna A/B Benchmarking**: Structured workflow to compare two antennas by holding conditions identical and generating statistical reports.
- **Data Logging**: Output logs in CSV, JSON, or human-readable formats via the Serial port.
- **Serial CLI**: Interact with the toolkit using clean, simple text commands over a serial monitor.

## Architecture

ESP32 RF Lab is designed using a modular C++ architecture to maintain separation of concerns:
- **Hardware Abstraction**: Determines chip model and capabilities.
- **Wi-Fi Engine**: Handles network connections and scanning.
- **Measurement Engine**: Collects samples and computes statistical metrics.
- **Antenna Benchmark Engine**: Orchestrates A/B test sequences.
- **Logging Engine**: Handles formatted data output.
- **Serial CLI**: Processes user commands interactively.

## Setup & Installation

### Requirements
- VSCode with PlatformIO extension or PlatformIO Core CLI.
- A supported ESP32 development board.

### Building and Flashing
1. Clone this repository.
2. Open the project in PlatformIO.
3. Build for your specific target. For example, using the CLI:
   ```bash
   pio run -e esp32dev
   ```
   (Replace `esp32dev` with your target environment defined in `platformio.ini`, e.g., `esp32s3`, `esp32c3`).
4. Flash the board:
   ```bash
   pio run -e esp32dev -t upload
   ```

## CLI Usage

Connect to the ESP32 over Serial using a baud rate of `115200`. Use the following commands to control the tool:

| Command | Description |
|---|---|
| `HELP` | Show the list of available commands. |
| `INFO` | Display hardware information (model, revision, cores, SDK). |
| `SCAN` | Perform a Wi-Fi network scan and print results. |
| `CONNECT <ssid> [pass]` | Connect to a specified Wi-Fi network. |
| `DISCONNECT` | Disconnect from the current network. |
| `STATUS` | Show current connection status and IP address. |
| `RSSI` | Show the immediate RSSI value (requires connection). |
| `ANTENNA A` | Run a 10-second RSSI benchmark for "Antenna A". |
| `ANTENNA B` | Run a 10-second RSSI benchmark for "Antenna B". |
| `COMPARE` | Compare the results of the Antenna A and Antenna B benchmarks. |
| `LOG START` | Begin logging RSSI data every second in CSV format. |
| `LOG STOP` | Stop the background logger. |

## Antenna Testing Methodology

When performing an A/B test between two antennas:
1. Ensure the ESP32 is placed in the exact same location and orientation for both tests.
2. Connect to a stable Wi-Fi network using the `CONNECT` command.
3. Attach the first antenna. Run `ANTENNA A`. Wait for the test to complete.
4. Carefully power down (if necessary), attach the second antenna, power up, connect, and ensure it is placed exactly as before. Run `ANTENNA B`.
5. Run `COMPARE` to view the statistical differences in signal quality.

**Note**: Do not rely on instantaneous RSSI for conclusions. Evaluate the average, minimum, and variance over the measurement duration.
