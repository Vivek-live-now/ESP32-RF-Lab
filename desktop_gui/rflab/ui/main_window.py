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
        self.lbl_live_rssi.setStyleSheet("font-size: 24px; font-weight: bold;")

        self.lbl_avg_rssi = QLabel("Avg: --")
        self.lbl_min_rssi = QLabel("Min: --")
        self.lbl_max_rssi = QLabel("Max: --")

        stats_layout.addWidget(self.lbl_live_rssi)
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
        ant_layout.addWidget(QLabel("Antenna Lab (Comparison implemented via DB export for now)"))
        self.tabs.addTab(self.tab_antenna, "Antenna Lab")

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

            # Plot
            self.plot_time.append(ts)
            self.plot_rssi.append(rssi)

            # Keep last 500 points for performance
            if len(self.plot_time) > 500:
                self.plot_time = self.plot_time[-500:]
                self.plot_rssi = self.plot_rssi[-500:]

            self.plot_curve.setData(self.plot_time, self.plot_rssi)

def run():
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())

if __name__ == "__main__":
    run()
