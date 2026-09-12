class BaseTransport:
    def connect(self, port: str):
        pass

    def disconnect(self):
        pass

    def is_connected(self) -> bool:
        return False

    def read_line(self) -> str:
        return ""

    def write_line(self, line: str):
        pass

    def get_available_ports(self) -> list:
        return []
