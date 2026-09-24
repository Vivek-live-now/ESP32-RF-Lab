import time
import random
from .base import BaseTransport

class SimulatedTransport(BaseTransport):
    def __init__(self):
        self.connected = False
        self.port_name = "SIMULATOR"
        self.telemetry_active = False
        self.rate_hz = 1
        self.last_emit = 0
        self.seq = 0
        self.pending_lines = []

        # Simulation states
        self.drop_prob = 0.0
        self.corrupt_prob = 0.0

    def get_available_ports(self):
        return ["SIMULATOR", "SIMULATOR_NOISY"]

    def connect(self, port: str):
        self.port_name = port
        self.connected = True
        self.telemetry_active = False
        self.last_emit = time.time()
        self.seq = 0
        self.pending_lines.clear()

        if port == "SIMULATOR_NOISY":
            self.drop_prob = 0.05
            self.corrupt_prob = 0.05
        else:
            self.drop_prob = 0.0
            self.corrupt_prob = 0.0

        return True

    def disconnect(self):
        self.connected = False
        self.telemetry_active = False
        self.pending_lines.clear()

    def is_connected(self) -> bool:
        return self.connected

    def _calc_checksum(self, data: str) -> int:
        c = 0
        for char in data:
            c ^= ord(char)
        return c

    def read_line(self) -> str:
        if not self.connected:
            return ""

        if self.pending_lines:
            return self.pending_lines.pop(0)

        if self.telemetry_active:
            now = time.time()
            interval = 1.0 / self.rate_hz
            if now - self.last_emit >= interval:
                self.last_emit = now

                # Simulate packet drop
                if random.random() < self.drop_prob:
                    self.seq += 1
                    return ""

                # DATA,seq,timestamp,rssi,channel,latency,loss,temp
                ts = int(now * 1000) % 10000000
                rssi = -58 + int(random.gauss(0, 3))
                channel = 6
                latency = max(1.0, 8.5 + random.gauss(0, 1.5))
                loss = max(0.0, 0.2 + random.gauss(0, 0.2))
                temp = 42.0 + random.gauss(0, 0.5)

                payload = f"DATA,{self.seq},{ts},{rssi},{channel},{latency:.2f},{loss:.2f},{temp:.1f}"
                self.seq += 1

                checksum = self._calc_checksum(payload)

                # Simulate corruption
                if random.random() < self.corrupt_prob:
                    payload = payload.replace("DATA", "DATX") # Corrupt payload, checksum fails

                return f"{payload},{checksum:02X}"

        time.sleep(0.01) # Prevent busy waiting
        return ""

    def write_line(self, line: str):
        if not self.connected:
            return

        cmd = line.strip().upper()
        if cmd.startswith("STREAM START"):
            parts = cmd.split()
            if len(parts) > 2:
                try:
                    self.rate_hz = int(parts[2])
                except:
                    self.rate_hz = 1
            else:
                self.rate_hz = 1
            self.telemetry_active = True
            self.pending_lines.append(f"ACK_STREAM_START,{self.rate_hz}")
        elif cmd == "STREAM STOP":
            self.telemetry_active = False
            self.pending_lines.append("ACK_STREAM_STOP")
        elif cmd == "ANTENNA A":
            # Simulate Antenna A result
            self.pending_lines.append("Testing Antenna A...")
            self.pending_lines.append("ANTENNA_RES,A,20,-65,-50,-57.4,3.20")
        elif cmd == "ANTENNA B":
            # Simulate Antenna B result (stronger)
            self.pending_lines.append("Testing Antenna B...")
            self.pending_lines.append("ANTENNA_RES,B,20,-58,-44,-50.8,2.75")
        elif cmd == "COMPARE":
            self.pending_lines.append("=== Antenna Comparison ===")
            self.pending_lines.append("Antenna B is +6.60 dBm stronger on average.")
        elif cmd.startswith("PING"):
            self.pending_lines.append("PING_RES,192.168.1.1,5,5,0.0,7.85")
