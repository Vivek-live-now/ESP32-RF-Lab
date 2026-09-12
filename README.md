# ESP32 RF Lab

ESP32 RF Lab is a portable Wi-Fi and RF experimentation and benchmarking toolkit for the ESP32 family. It is designed to evaluate antenna performance, signal strength, and connection quality.

**Core Philosophy: Measure on ESP32. Analyze on PC.**
The ESP32 firmware handles the raw RF data collection, while the robust desktop GUI application (PySide6 + PyQtGraph) handles complex calculations, moving averages, databases, and charting.

## Features

- **Wi-Fi Network Discovery**: Scan for networks and display signal strength, channel, and BSSID.
- **Hardware Agnostic**: Firmware supports ESP32, ESP32-S2, ESP32-S3, ESP32-C3.
- **Antenna A/B Benchmarking**: Structured workflow to compare two antennas by holding conditions identical and generating statistical reports.
- **Serial CLI**: Interact with the toolkit using clean, simple text commands over a serial monitor.
- **High-End Desktop GUI**: A robust PySide6 application with real-time `pyqtgraph` charts, structured JSON/Checksummed telemetry, SQLite session storage, and CSV/JSON exporting.

## Setup & Installation

### 1. ESP32 Firmware
1. Open the repository root in PlatformIO (VSCode).
2. Build for your specific target (e.g., `pio run -e esp32dev`).
3. Flash the board (`pio run -e esp32dev -t upload`).

### 2. Desktop GUI
The GUI requires Python 3.10+.
1. Navigate to the GUI folder:
   ```bash
   cd desktop_gui
   ```
2. Install the requirements:
   ```bash
   pip install -r requirements.txt
   ```
3. Run the application:
   ```bash
   python main.py
   ```
   *(Note: You can select "SIMULATOR" from the ports dropdown to test the GUI without an ESP32 connected!)*

## CLI Usage (Firmware)

Connect to the ESP32 over Serial using a baud rate of `115200`. Use the following commands to control the tool manually:

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
| `STREAM START [rate]` | Starts the structured framed telemetry protocol for the GUI (rate in Hz: 1, 2, 5, 10). |
| `STREAM STOP` | Stops the structured GUI telemetry. |

## Antenna Testing Methodology

When performing an A/B test between two antennas:
1. Ensure the ESP32 is placed in the exact same location and orientation for both tests.
2. Connect to a stable Wi-Fi network using the `CONNECT` command.
3. Attach the first antenna. Run `ANTENNA A`. Wait for the test to complete.
4. Carefully power down (if necessary), attach the second antenna, power up, connect, and ensure it is placed exactly as before. Run `ANTENNA B`.
5. Run `COMPARE` to view the statistical differences in signal quality, or stream the data directly to the **Desktop GUI** for persistent storage and graphing.

**Note**: Do not rely on instantaneous RSSI for conclusions. Evaluate the average, minimum, and variance over the measurement duration.
