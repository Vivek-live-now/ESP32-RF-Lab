from typing import Optional, Dict, Any

class ProtocolParser:
    """
    Parses incoming lines from the ESP32 firmware.
    Expected format for telemetry: DATA,seq,timestamp,rssi,channel,latency,loss,checksum
    """

    @staticmethod
    def _verify_checksum(line: str) -> bool:
        if ',' not in line:
            return False

        last_comma = line.rfind(',')
        payload = line[:last_comma]
        checksum_hex = line[last_comma+1:]

        try:
            expected = int(checksum_hex, 16)
        except ValueError:
            return False

        calc = 0
        for char in payload:
            calc ^= ord(char)

        return calc == expected

    @staticmethod
    def parse_line(line: str) -> Optional[Dict[str, Any]]:
        line = line.strip()
        if not line:
            return None

        # Telemetry format
        if line.startswith("DATA,"):
            if not ProtocolParser._verify_checksum(line):
                return {"type": "error", "msg": "Checksum failed", "raw": line}

            parts = line.split(',')
            if len(parts) != 8:
                return {"type": "error", "msg": "Malformed telemetry", "raw": line}

            try:
                return {
                    "type": "telemetry",
                    "seq": int(parts[1]),
                    "timestamp_ms": int(parts[2]),
                    "rssi": int(parts[3]),
                    "channel": int(parts[4]),
                    "latency": float(parts[5]),
                    "loss": float(parts[6])
                }
            except ValueError:
                return {"type": "error", "msg": "Data type parsing failed", "raw": line}

        # Command ACKs
        if line.startswith("ACK_"):
            return {"type": "ack", "raw": line}

        # Unknown/Human readable
        return {"type": "log", "msg": line}
