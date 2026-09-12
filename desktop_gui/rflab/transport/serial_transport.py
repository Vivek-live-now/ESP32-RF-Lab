import serial
import serial.tools.list_ports
import time
from .base import BaseTransport

class SerialTransport(BaseTransport):
    def __init__(self, baudrate=115200, timeout=1.0):
        self.baudrate = baudrate
        self.timeout = timeout
        self.ser = None

    def get_available_ports(self):
        return [port.device for port in serial.tools.list_ports.comports()]

    def connect(self, port: str):
        try:
            self.ser = serial.Serial(port, self.baudrate, timeout=self.timeout)
            time.sleep(1) # Allow reset
            return True
        except serial.SerialException:
            return False

    def disconnect(self):
        if self.ser and self.ser.is_open:
            self.ser.close()
            self.ser = None

    def is_connected(self) -> bool:
        return self.ser is not None and self.ser.is_open

    def read_line(self) -> str:
        if not self.is_connected():
            return ""
        try:
            line = self.ser.readline()
            if line:
                return line.decode('utf-8', errors='replace').strip()
            return ""
        except serial.SerialException:
            self.disconnect()
            return ""

    def write_line(self, line: str):
        if self.is_connected():
            try:
                self.ser.write((line + '\n').encode('utf-8'))
            except serial.SerialException:
                self.disconnect()
