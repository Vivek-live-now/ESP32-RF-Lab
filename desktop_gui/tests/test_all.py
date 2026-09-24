try:
    import pytest
except ImportError:
    pytest = None
import os
import sys
import tempfile
import pathlib

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from rflab.transport.protocol import ProtocolParser
from rflab.storage.database import DatabaseManager
from rflab.analysis.engine import AnalysisEngine

def test_protocol_parser_valid():
    payload = "DATA,1,1000,-60,6,1.0,0.0"
    calc = 0
    for char in payload: calc ^= ord(char)
    line = f"{payload},{calc:02X}"

    res = ProtocolParser.parse_line(line)
    assert res is not None
    assert res["type"] == "telemetry"
    assert res["seq"] == 1
    assert res["rssi"] == -60

def test_protocol_parser_invalid():
    res = ProtocolParser.parse_line("DATA,1,1000,-60,6,1.0,0.0,FF")
    assert res["type"] == "error"

def test_protocol_parser_valid_with_temp():
    payload = "DATA,2,2000,-55,1,8.5,0.0,41.2"
    calc = 0
    for char in payload: calc ^= ord(char)
    line = f"{payload},{calc:02X}"

    res = ProtocolParser.parse_line(line)
    assert res is not None
    assert res["type"] == "telemetry"
    assert res["seq"] == 2
    assert res["rssi"] == -55
    assert res["channel"] == 1
    assert res["latency"] == 8.5
    assert res["loss"] == 0.0
    assert res["temp"] == 41.2

def test_protocol_parser_antenna_result():
    line = "ANTENNA_RES,A,20,-65,-50,-57.4,3.20"
    res = ProtocolParser.parse_line(line)
    assert res is not None
    assert res["type"] == "antenna_result"
    assert res["antenna"] == "A"
    assert res["samples"] == 20
    assert res["min"] == -65
    assert res["max"] == -50
    assert res["avg"] == -57.4
    assert res["stddev"] == 3.20

def test_protocol_parser_ping_result():
    line = "PING_RES,192.168.1.1,5,5,0.0,6.5"
    res = ProtocolParser.parse_line(line)
    assert res is not None
    assert res["type"] == "ping_result"
    assert res["target"] == "192.168.1.1"
    assert res["sent"] == 5
    assert res["recv"] == 5
    assert res["loss"] == 0.0
    assert res["latency"] == 6.5

def test_database_manager(tmp_path):
    db_path = tmp_path / "test.db"
    db = DatabaseManager(str(db_path))

    sid = db.create_session("Test", {})
    db.insert_telemetry(sid, {"timestamp_ms": 100, "rssi": -50, "channel": 6, "latency": 1.0, "loss": 0.0})
    db.insert_telemetry(sid, {"timestamp_ms": 200, "rssi": -55, "channel": 6, "latency": 1.0, "loss": 0.0})

    data = db.get_session_data(sid)
    assert len(data) == 2
    assert data[0][1] == -50
    assert data[1][1] == -55

def test_analysis_engine():
    engine = AnalysisEngine(window_size=10)
    engine.add_sample(-40)
    engine.add_sample(-50)
    engine.add_sample(-60)

    stats = engine.get_stats()
    assert stats["min"] == -60
    assert stats["max"] == -40
    assert stats["avg"] == -50.0
    assert stats["stddev"] > 0.0

def test_analysis_compare_sessions():
    data_a = [(100, -60, 6, 10.0, 0.0), (200, -64, 6, 12.0, 0.0)]
    data_b = [(100, -50, 6, 5.0, 0.0), (200, -52, 6, 6.0, 0.0)]

    comp = AnalysisEngine.compare_sessions(data_a, data_b)
    assert comp["A"]["avg"] == -62.0
    assert comp["B"]["avg"] == -51.0
    assert comp["Diff_Avg"] == 11.0 # B is 11 dBm stronger than A

if __name__ == "__main__":
    print("Running RF Lab Desktop unit tests...")
    test_protocol_parser_valid()
    test_protocol_parser_invalid()
    test_protocol_parser_valid_with_temp()
    test_protocol_parser_antenna_result()
    test_protocol_parser_ping_result()
    with tempfile.TemporaryDirectory() as td:
        test_database_manager(pathlib.Path(td))
    test_analysis_engine()
    test_analysis_compare_sessions()
    print("SUCCESS: All unit tests passed!")


