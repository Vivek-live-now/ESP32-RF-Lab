# ESP32 RF Lab

ESP32 RF Lab is a portable Wi-Fi and RF experimentation and benchmarking toolkit for the ESP32 family. It is designed to evaluate antenna performance, signal strength, and connection quality.

**Core Philosophy: Measure on ESP32. Analyze on PC.**
The ESP32 firmware handles the raw RF data collection, while the robust desktop GUI application (PySide6 + PyQtGraph) handles complex calculations, moving averages, databases, and charting.

## Features

- **Wi-Fi Network Discovery**: Scan for networks and display signal strength, channel, and BSSID.
- **Hardware Agnostic**: Firmware supports ESP32, ESP32-S2, ESP32-S3, ESP32-C3.
- **Antenna A/B Benchmarking**: Structured workflow to compare two antennas by holding conditions identical and generating statistical reports.
- **Real Physical RF & Network Metrics**:
  - Direct RSSI statistics (Min, Max, Mean, Standard Deviation / Jitter)
  - Active Wi-Fi Channel reporting
  - Onboard internal chip temperature (°C)
  - True ICMP Gateway Ping round-trip latency (ms) and Packet Loss rate (%)
  - Link Health & Stability scoring
- **Serial CLI**: Interact with the toolkit using clean, simple text commands over a serial monitor.
- **High-End Desktop GUI**: A lightweight PySide6 application with real-time `pyqtgraph` charts, structured checksummed telemetry, SQLite session storage, CSV exporting, and an interactive **Antenna Lab** comparison interface.

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
| `INFO` | Display hardware info (model, revision, cores, SDK, and internal temperature). |
| `SCAN` | Perform a Wi-Fi network scan and print results. |
| `CONNECT <ssid> [pass]` | Connect to a specified Wi-Fi network. |
| `DISCONNECT` | Disconnect from the current network. |
| `STATUS` | Show connection status, IP, gateway, channel, RSSI, and temperature. |
| `RSSI` | Show immediate RSSI value and active channel. |
| `PING [ip/host]` | Send ICMP echo requests to gateway or custom host; reports latency and packet loss. |
| `STABILITY [sec]` | Run RSSI jitter & gateway ping analysis; generates Link Health score (0-100). |
| `ANTENNA A` | Run a 10-second RSSI benchmark for "Antenna A". |
| `ANTENNA B` | Run a 10-second RSSI benchmark for "Antenna B". |
| `COMPARE` | Compare the results of the Antenna A and Antenna B benchmarks. |
| `STREAM START [rate]` | Starts structured framed telemetry protocol for GUI (rate in Hz: 1, 2, 5, 10, 20). |
| `STREAM STOP` | Stops GUI telemetry streaming. |
| `LOG START` | Begin logging data in CSV format. |
| `LOG STOP` | Stop the background logger. |
| `THROUGHPUT` | Show 802.11n PHY speed capabilities and link limits. |

## Antenna Testing Methodology

When performing an A/B test between two antennas:
1. Ensure the ESP32 is placed in the exact same location and orientation for both tests.
2. Connect to a stable Wi-Fi network using the `CONNECT` command.
3. Attach the first antenna. Run `ANTENNA A`. Wait for the test to complete.
4. Carefully power down (if necessary), attach the second antenna, power up, connect, and ensure it is placed exactly as before. Run `ANTENNA B`.
5. Run `COMPARE` to view the statistical differences in signal quality, or stream the data directly to the **Desktop GUI** for persistent storage and graphing.

**Note**: Do not rely on instantaneous RSSI for conclusions. Evaluate the average, minimum, and variance over the measurement duration.
