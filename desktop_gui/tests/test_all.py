import pytest
import os
import sys

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
