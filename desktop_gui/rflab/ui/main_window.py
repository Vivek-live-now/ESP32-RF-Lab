import sys
import pyqtgraph as pg
from PySide6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QPushButton, QLabel, QComboBox, QTabWidget, QTableWidget,
    QTableWidgetItem, QGroupBox, QSpinBox
)
from PySide6.QtCore import QTimer, Slot, Signal, QObject, QThread

from rflab.transport.serial_transport import SerialTransport
from rflab.transport.simulated_transport import SimulatedTransport
from rflab.transport.protocol import ProtocolParser
from rflab.analysis.engine import AnalysisEngine
from rflab.storage.database import DatabaseManager

class SerialWorker(QThread):
    data_received = Signal(dict)

    def __init__(self, transport):
        super().__init__()
        self.transport = transport
        self.running = True

    def run(self):
        while self.running:
            line = self.transport.read_line()
            if line:
                parsed = ProtocolParser.parse_line(line)
                if parsed:
                    self.data_received.emit(parsed)
            else:
                self.msleep(10)

    def stop(self):
        self.running = False
        self.wait()

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("ESP32 RF Lab - Desktop GUI")
        self.resize(1000, 700)

        # State
        self.db = DatabaseManager("rflab_sessions.db")
        self.transport = SerialTransport() # Default, we'll allow mock switching
        self.analysis = AnalysisEngine()
        self.current_session_id = None
        self.worker = None

        # Chart Data
        self.plot_time = []
        self.plot_rssi = []

        self.init_ui()
        self.refresh_ports()

    def init_ui(self):
        central = QWidget()
        self.setCentralWidget(central)
        main_layout = QVBoxLayout(central)

        # --- Top Bar: Connection ---
        conn_group = QGroupBox("Device Connection")
        conn_layout = QHBoxLayout()

        self.port_combo = QComboBox()
        self.btn_refresh = QPushButton("Refresh")
        self.btn_refresh.clicked.connect(self.refresh_ports)

        self.btn_connect = QPushButton("Connect")
        self.btn_connect.clicked.connect(self.toggle_connection)

        self.lbl_status = QLabel("Disconnected")
        self.lbl_status.setStyleSheet("color: red; font-weight: bold;")

        self.btn_mock = QPushButton("Use Simulator")
        self.btn_mock.setCheckable(True)
        self.btn_mock.clicked.connect(self.toggle_simulator)

        conn_layout.addWidget(QLabel("Port:"))
        conn_layout.addWidget(self.port_combo)
        conn_layout.addWidget(self.btn_refresh)
        conn_layout.addWidget(self.btn_connect)
        conn_layout.addWidget(self.btn_mock)
        conn_layout.addStretch()
        conn_layout.addWidget(self.lbl_status)
        conn_group.setLayout(conn_layout)
        main_layout.addWidget(conn_group)

        # --- Tabs ---
        self.tabs = QTabWidget()
        main_layout.addWidget(self.tabs)

        # Tab 1: Live Dashboard
        self.tab_dash = QWidget()
        dash_layout = QVBoxLayout(self.tab_dash)

        # Stats Row
        stats_layout = QHBoxLayout()
        self.lbl_live_rssi = QLabel("RSSI: -- dBm")
        self.lbl_live_rssi.setStyleSheet("font-size: 24px; font-weight: bold; color: #1a73e8;")

        self.lbl_channel = QLabel("Ch: --")
        self.lbl_latency = QLabel("Latency: -- ms")
        self.lbl_loss = QLabel("Loss: --%")
        self.lbl_temp = QLabel("Temp: --°C")
        self.lbl_avg_rssi = QLabel("Avg: --")
        self.lbl_min_rssi = QLabel("Min: --")
        self.lbl_max_rssi = QLabel("Max: --")

        stats_layout.addWidget(self.lbl_live_rssi)
        stats_layout.addSpacing(20)
        stats_layout.addWidget(self.lbl_channel)
        stats_layout.addWidget(self.lbl_latency)
        stats_layout.addWidget(self.lbl_loss)
        stats_layout.addWidget(self.lbl_temp)
        stats_layout.addStretch()
        stats_layout.addWidget(self.lbl_avg_rssi)
        stats_layout.addWidget(self.lbl_min_rssi)
        stats_layout.addWidget(self.lbl_max_rssi)
        dash_layout.addLayout(stats_layout)

        # Graph
        pg.setConfigOptions(antialias=True)
        self.plot_widget = pg.PlotWidget()
        self.plot_widget.setBackground('w')
        self.plot_widget.setTitle("Live RSSI", color="b")
        self.plot_widget.setLabel('left', 'RSSI', units='dBm')
        self.plot_widget.setLabel('bottom', 'Time', units='s')
        self.plot_widget.showGrid(x=True, y=True)
        self.plot_curve = self.plot_widget.plot(pen=pg.mkPen(color='b', width=2))
        dash_layout.addWidget(self.plot_widget)

        # Controls Row
        ctrl_layout = QHBoxLayout()
        self.btn_stream_start = QPushButton("Start Test Stream")
        self.btn_stream_start.clicked.connect(self.start_stream)
        self.btn_stream_stop = QPushButton("Stop Stream")
        self.btn_stream_stop.clicked.connect(self.stop_stream)

        self.rate_spin = QSpinBox()
        self.rate_spin.setRange(1, 20)
        self.rate_spin.setValue(10)

        ctrl_layout.addWidget(QLabel("Rate (Hz):"))
        ctrl_layout.addWidget(self.rate_spin)
        ctrl_layout.addWidget(self.btn_stream_start)
        ctrl_layout.addWidget(self.btn_stream_stop)
        ctrl_layout.addStretch()
        dash_layout.addLayout(ctrl_layout)

        self.tabs.addTab(self.tab_dash, "Live Dashboard")

        # Tab 2: Antenna Lab
        self.tab_antenna = QWidget()
        ant_layout = QVBoxLayout(self.tab_antenna)

        bench_boxes_layout = QHBoxLayout()

        # Antenna A Box
        box_a = QGroupBox("Antenna A (Baseline)")
        box_a_layout = QVBoxLayout(box_a)
        self.lbl_ant_a_status = QLabel("Status: Ready to test")
        self.lbl_ant_a_stats = QLabel("Avg: -- dBm | Min: -- | Max: -- | StdDev: --")
        self.btn_test_ant_a = QPushButton("Run Antenna A Benchmark (10s)")
        self.btn_test_ant_a.clicked.connect(lambda: self.run_antenna_benchmark("A"))
        box_a_layout.addWidget(self.lbl_ant_a_status)
        box_a_layout.addWidget(self.lbl_ant_a_stats)
        box_a_layout.addWidget(self.btn_test_ant_a)
        bench_boxes_layout.addWidget(box_a)

        # Antenna B Box
        box_b = QGroupBox("Antenna B (Candidate)")
        box_b_layout = QVBoxLayout(box_b)
        self.lbl_ant_b_status = QLabel("Status: Ready to test")
        self.lbl_ant_b_stats = QLabel("Avg: -- dBm | Min: -- | Max: -- | StdDev: --")
        self.btn_test_ant_b = QPushButton("Run Antenna B Benchmark (10s)")
        self.btn_test_ant_b.clicked.connect(lambda: self.run_antenna_benchmark("B"))
        box_b_layout.addWidget(self.lbl_ant_b_status)
        box_b_layout.addWidget(self.lbl_ant_b_stats)
        box_b_layout.addWidget(self.btn_test_ant_b)
        bench_boxes_layout.addWidget(box_b)

        ant_layout.addLayout(bench_boxes_layout)

        # Comparison Section
        comp_group = QGroupBox("A/B Comparative Analysis")
        comp_layout = QVBoxLayout(comp_group)

        self.lbl_verdict = QLabel("Run benchmark on both Antenna A and B to generate comparison.")
        self.lbl_verdict.setStyleSheet("font-weight: bold; font-size: 14px; color: #333;")
        comp_layout.addWidget(self.lbl_verdict)

        self.tbl_compare = QTableWidget(4, 4)
        self.tbl_compare.setHorizontalHeaderLabels(["Metric", "Antenna A", "Antenna B", "Delta (B - A)"])
        self.tbl_compare.setItem(0, 0, QTableWidgetItem("Average RSSI"))
        self.tbl_compare.setItem(1, 0, QTableWidgetItem("Min RSSI"))
        self.tbl_compare.setItem(2, 0, QTableWidgetItem("Max RSSI"))
        self.tbl_compare.setItem(3, 0, QTableWidgetItem("StdDev (Jitter)"))
        comp_layout.addWidget(self.tbl_compare)

        self.btn_calc_compare = QPushButton("Calculate Comparison")
        self.btn_calc_compare.clicked.connect(self.calculate_antenna_comparison)
        comp_layout.addWidget(self.btn_calc_compare)

        ant_layout.addWidget(comp_group)
        self.tabs.addTab(self.tab_antenna, "Antenna Lab")

        self.antenna_a_stats = None
        self.antenna_b_stats = None

    @Slot()
    def toggle_simulator(self):
        if self.btn_mock.isChecked():
            self.transport = SimulatedTransport()
        else:
            self.transport = SerialTransport()
        self.refresh_ports()

    @Slot()
    def refresh_ports(self):
        self.port_combo.clear()
        self.port_combo.addItems(self.transport.get_available_ports())

    @Slot()
    def toggle_connection(self):
        if self.transport.is_connected():
            self.stop_stream()
            if self.worker:
                self.worker.stop()
            self.transport.disconnect()
            self.btn_connect.setText("Connect")
            self.lbl_status.setText("Disconnected")
            self.lbl_status.setStyleSheet("color: red; font-weight: bold;")
            self.port_combo.setEnabled(True)
        else:
            port = self.port_combo.currentText()
            if port and self.transport.connect(port):
                self.btn_connect.setText("Disconnect")
                self.lbl_status.setText("Connected")
                self.lbl_status.setStyleSheet("color: green; font-weight: bold;")
                self.port_combo.setEnabled(False)

                # Start background thread
                self.worker = SerialWorker(self.transport)
                self.worker.data_received.connect(self.handle_data)
                self.worker.start()

    @Slot()
    def start_stream(self):
        if self.transport.is_connected():
            rate = self.rate_spin.value()
            self.current_session_id = self.db.create_session("LiveStream", {"rate": rate})
            self.analysis.reset()
            self.plot_time.clear()
            self.plot_rssi.clear()
            self.transport.write_line(f"STREAM START {rate}")

    @Slot()
    def stop_stream(self):
        if self.transport.is_connected():
            self.transport.write_line("STREAM STOP")
            if self.current_session_id:
                self.db.end_session(self.current_session_id)
                self.current_session_id = None

    def run_antenna_benchmark(self, antenna: str):
        if not self.transport.is_connected():
            return
        if antenna == "A":
            self.lbl_ant_a_status.setText("Status: Benchmarking Antenna A (10s)...")
            self.lbl_ant_a_status.setStyleSheet("color: blue; font-weight: bold;")
            self.transport.write_line("ANTENNA A")
        elif antenna == "B":
            self.lbl_ant_b_status.setText("Status: Benchmarking Antenna B (10s)...")
            self.lbl_ant_b_status.setStyleSheet("color: blue; font-weight: bold;")
            self.transport.write_line("ANTENNA B")

    def calculate_antenna_comparison(self):
        if not self.antenna_a_stats or not self.antenna_b_stats:
            self.lbl_verdict.setText("Please run benchmarks for BOTH Antenna A and Antenna B first.")
            self.lbl_verdict.setStyleSheet("color: red; font-weight: bold;")
            return

        a = self.antenna_a_stats
        b = self.antenna_b_stats

        diff_avg = b["avg"] - a["avg"]
        diff_min = b["min"] - a["min"]
        diff_max = b["max"] - a["max"]
        diff_std = b["stddev"] - a["stddev"]

        # Populate table
        self.tbl_compare.setItem(0, 1, QTableWidgetItem(f"{a['avg']:.2f} dBm"))
        self.tbl_compare.setItem(0, 2, QTableWidgetItem(f"{b['avg']:.2f} dBm"))
        self.tbl_compare.setItem(0, 3, QTableWidgetItem(f"{diff_avg:+.2f} dBm"))

        self.tbl_compare.setItem(1, 1, QTableWidgetItem(f"{a['min']} dBm"))
        self.tbl_compare.setItem(1, 2, QTableWidgetItem(f"{b['min']} dBm"))
        self.tbl_compare.setItem(1, 3, QTableWidgetItem(f"{diff_min:+d} dBm"))

        self.tbl_compare.setItem(2, 1, QTableWidgetItem(f"{a['max']} dBm"))
        self.tbl_compare.setItem(2, 2, QTableWidgetItem(f"{b['max']} dBm"))
        self.tbl_compare.setItem(2, 3, QTableWidgetItem(f"{diff_max:+d} dBm"))

        self.tbl_compare.setItem(3, 1, QTableWidgetItem(f"{a['stddev']:.2f}"))
        self.tbl_compare.setItem(3, 2, QTableWidgetItem(f"{b['stddev']:.2f}"))
        self.tbl_compare.setItem(3, 3, QTableWidgetItem(f"{diff_std:+.2f}"))

        if diff_avg > 0:
            self.lbl_verdict.setText(f"VERDICT: Antenna B is STRONGER by {diff_avg:+.2f} dBm on average.")
            self.lbl_verdict.setStyleSheet("color: green; font-weight: bold; font-size: 14px;")
        elif diff_avg < 0:
            self.lbl_verdict.setText(f"VERDICT: Antenna A is STRONGER by {abs(diff_avg):.2f} dBm on average.")
            self.lbl_verdict.setStyleSheet("color: #d93025; font-weight: bold; font-size: 14px;")
        else:
            self.lbl_verdict.setText("VERDICT: Both antennas showed identical average signal strength.")
            self.lbl_verdict.setStyleSheet("color: #333; font-weight: bold; font-size: 14px;")

    @Slot(dict)
    def handle_data(self, data):
        if data["type"] == "telemetry":
            rssi = data["rssi"]
            ts = data["timestamp_ms"] / 1000.0 # to seconds

            # DB Storage
            if self.current_session_id:
                self.db.insert_telemetry(self.current_session_id, data)

            # Analysis
            self.analysis.add_sample(rssi)
            stats = self.analysis.get_stats()

            # UI Update
            self.lbl_live_rssi.setText(f"RSSI: {rssi} dBm")
            self.lbl_avg_rssi.setText(f"Avg: {stats['avg']:.1f}")
            self.lbl_min_rssi.setText(f"Min: {stats['min']}")
            self.lbl_max_rssi.setText(f"Max: {stats['max']}")

            if "channel" in data:
                self.lbl_channel.setText(f"Ch: {data['channel']}")
            if "latency" in data and data["latency"] > 0:
                self.lbl_latency.setText(f"Latency: {data['latency']:.1f} ms")
            if "loss" in data:
                self.lbl_loss.setText(f"Loss: {data['loss']:.1f}%")
            if "temp" in data:
                self.lbl_temp.setText(f"Temp: {data['temp']:.1f}°C")

            # Plot
            self.plot_time.append(ts)
            self.plot_rssi.append(rssi)

            # Keep last 500 points for performance
            if len(self.plot_time) > 500:
                self.plot_time = self.plot_time[-500:]
                self.plot_rssi = self.plot_rssi[-500:]

            self.plot_curve.setData(self.plot_time, self.plot_rssi)

        elif data["type"] == "antenna_result":
            ant = data["antenna"].upper()
            if ant == "A":
                self.antenna_a_stats = data
                self.lbl_ant_a_status.setText("Status: Completed")
                self.lbl_ant_a_status.setStyleSheet("color: green; font-weight: bold;")
                self.lbl_ant_a_stats.setText(
                    f"Avg: {data['avg']:.1f} dBm | Min: {data['min']} | Max: {data['max']} | StdDev: {data['stddev']:.2f}"
                )
            elif ant == "B":
                self.antenna_b_stats = data
                self.lbl_ant_b_status.setText("Status: Completed")
                self.lbl_ant_b_status.setStyleSheet("color: green; font-weight: bold;")
                self.lbl_ant_b_stats.setText(
                    f"Avg: {data['avg']:.1f} dBm | Min: {data['min']} | Max: {data['max']} | StdDev: {data['stddev']:.2f}"
                )
            if self.antenna_a_stats and self.antenna_b_stats:
                self.calculate_antenna_comparison()

def run():
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())

if __name__ == "__main__":
    run()
