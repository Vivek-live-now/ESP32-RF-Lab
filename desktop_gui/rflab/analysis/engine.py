import math

class AnalysisEngine:
    def __init__(self, window_size=100):
        self.window_size = window_size
        self.rssi_buffer = []

        # Stats
        self.min_rssi = float('inf')
        self.max_rssi = float('-inf')
        self.sum_rssi = 0
        self.count = 0

    def reset(self):
        self.rssi_buffer.clear()
        self.min_rssi = float('inf')
        self.max_rssi = float('-inf')
        self.sum_rssi = 0
        self.count = 0

    def add_sample(self, rssi: int):
        self.rssi_buffer.append(rssi)
        if len(self.rssi_buffer) > self.window_size:
            self.rssi_buffer.pop(0)

        if rssi < self.min_rssi: self.min_rssi = rssi
        if rssi > self.max_rssi: self.max_rssi = rssi
        self.sum_rssi += rssi
        self.count += 1

    def get_stats(self) -> dict:
        if self.count == 0:
            return {
                "avg": 0.0, "min": 0, "max": 0,
                "stddev": 0.0, "moving_avg": 0.0
            }

        avg = self.sum_rssi / self.count

        # Moving avg
        if len(self.rssi_buffer) > 0:
            moving_avg = sum(self.rssi_buffer) / len(self.rssi_buffer)
        else:
            moving_avg = 0.0

        # Calculate stddev of the moving window
        if len(self.rssi_buffer) > 1:
            mean = sum(self.rssi_buffer) / len(self.rssi_buffer)
            variance = sum((x - mean) ** 2 for x in self.rssi_buffer) / len(self.rssi_buffer)
            stddev = math.sqrt(variance)
        else:
            stddev = 0.0

        return {
            "avg": avg,
            "min": self.min_rssi,
            "max": self.max_rssi,
            "stddev": stddev,
            "moving_avg": moving_avg
        }

    @staticmethod
    def compare_sessions(data_a: list, data_b: list) -> dict:
        """
        Expects list of tuples/lists: [(ts, rssi, ch, lat, loss, [temp]), ...]
        """
        def calc_for(data):
            if not data:
                return {
                    "count": 0, "avg": 0.0, "min": 0, "max": 0,
                    "stddev": 0.0, "latency": 0.0, "loss": 0.0
                }
            rssis = [r[1] for r in data]
            latencies = [r[3] for r in data if len(r) > 3]
            losses = [r[4] for r in data if len(r) > 4]

            avg_rssi = sum(rssis) / len(rssis)
            var_rssi = sum((x - avg_rssi) ** 2 for x in rssis) / len(rssis) if len(rssis) > 1 else 0.0
            stddev_rssi = math.sqrt(var_rssi)

            return {
                "count": len(rssis),
                "avg": round(avg_rssi, 2),
                "min": min(rssis),
                "max": max(rssis),
                "stddev": round(stddev_rssi, 2),
                "latency": round(sum(latencies) / len(latencies), 2) if latencies else 0.0,
                "loss": round(sum(losses) / len(losses), 2) if losses else 0.0
            }

        res_a = calc_for(data_a)
        res_b = calc_for(data_b)

        return {
            "A": res_a,
            "B": res_b,
            "Diff_Avg": round(res_b["avg"] - res_a["avg"], 2),
            "Diff_Min": res_b["min"] - res_a["min"],
            "Diff_Max": res_b["max"] - res_a["max"],
            "Diff_Loss": round(res_b["loss"] - res_a["loss"], 2)
        }
