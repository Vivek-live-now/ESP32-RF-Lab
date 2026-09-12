import numpy as np

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

        # Calculate stddev of the moving window to keep it fast
        if len(self.rssi_buffer) > 1:
            arr = np.array(self.rssi_buffer)
            stddev = np.std(arr)
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
        Expects list of tuples/lists: [(ts, rssi, ch, lat, loss), ...]
        """
        def calc_for(data):
            if not data: return {"avg": 0, "min": 0, "max": 0, "loss": 0}
            rssis = [r[1] for r in data]
            losses = [r[4] for r in data]
            return {
                "avg": sum(rssis) / len(rssis),
                "min": min(rssis),
                "max": max(rssis),
                "loss": sum(losses) / len(losses)
            }

        res_a = calc_for(data_a)
        res_b = calc_for(data_b)

        return {
            "A": res_a,
            "B": res_b,
            "Diff_Avg": res_b["avg"] - res_a["avg"]
        }
