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

        if self.telemetry_active:
            now = time.time()
            interval = 1.0 / self.rate_hz
            if now - self.last_emit >= interval:
                self.last_emit = now

                # Simulate packet drop
                if random.random() < self.drop_prob:
                    self.seq += 1
                    return ""

                # DATA,seq,timestamp,rssi,channel,latency,loss
                ts = int(now * 1000) % 10000000
                rssi = -60 + int(random.gauss(0, 5))
                channel = 6
                latency = max(1.0, 10.0 + random.gauss(0, 2))
                loss = max(0.0, 0.5 + random.gauss(0, 0.5))

                payload = f"DATA,{self.seq},{ts},{rssi},{channel},{latency:.2f},{loss:.2f}"
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
        elif cmd == "STREAM STOP":
            self.telemetry_active = False
