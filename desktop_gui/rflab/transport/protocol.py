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
            if len(parts) not in (8, 9):
                return {"type": "error", "msg": "Malformed telemetry", "raw": line}

            try:
                data = {
                    "type": "telemetry",
                    "seq": int(parts[1]),
                    "timestamp_ms": int(parts[2]),
                    "rssi": int(parts[3]),
                    "channel": int(parts[4]),
                    "latency": float(parts[5]),
                    "loss": float(parts[6])
                }
                if len(parts) == 9:
                    data["temp"] = float(parts[7])
                return data
            except ValueError:
                return {"type": "error", "msg": "Data type parsing failed", "raw": line}

        # Benchmark results: ANTENNA_RES,<antenna>,<samples>,<min>,<max>,<avg>,<stddev>
        if line.startswith("ANTENNA_RES,"):
            parts = line.split(',')
            if len(parts) >= 7:
                try:
                    return {
                        "type": "antenna_result",
                        "antenna": parts[1],
                        "samples": int(parts[2]),
                        "min": int(parts[3]),
                        "max": int(parts[4]),
                        "avg": float(parts[5]),
                        "stddev": float(parts[6])
                    }
                except ValueError:
                    pass

        # Ping response: PING_RES,<ip>,<sent>,<recv>,<loss_pct>,<avg_latency>
        if line.startswith("PING_RES,"):
            parts = line.split(',')
            if len(parts) >= 6:
                try:
                    return {
                        "type": "ping_result",
                        "target": parts[1],
                        "sent": int(parts[2]),
                        "recv": int(parts[3]),
                        "loss": float(parts[4]),
                        "latency": float(parts[5])
                    }
                except ValueError:
                    pass

        # Command ACKs
        if line.startswith("ACK_"):
            return {"type": "ack", "raw": line}

        # Unknown/Human readable
        return {"type": "log", "msg": line}
