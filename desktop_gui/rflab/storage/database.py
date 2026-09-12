import sqlite3
import time
import json
import csv
from pathlib import Path

class DatabaseManager:
    def __init__(self, db_path="rflab_sessions.db"):
        self.db_path = db_path
        self._init_db()

    def _init_db(self):
        with sqlite3.connect(self.db_path) as conn:
            cursor = conn.cursor()
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS sessions (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    name TEXT NOT NULL,
                    start_time REAL NOT NULL,
                    end_time REAL,
                    metadata JSON
                )
            ''')
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS telemetry (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    session_id INTEGER,
                    timestamp_ms INTEGER,
                    rssi INTEGER,
                    channel INTEGER,
                    latency REAL,
                    loss REAL,
                    FOREIGN KEY(session_id) REFERENCES sessions(id)
                )
            ''')
            conn.commit()

    def create_session(self, name: str, metadata: dict) -> int:
        with sqlite3.connect(self.db_path) as conn:
            cursor = conn.cursor()
            cursor.execute(
                'INSERT INTO sessions (name, start_time, metadata) VALUES (?, ?, ?)',
                (name, time.time(), json.dumps(metadata))
            )
            return cursor.lastrowid

    def end_session(self, session_id: int):
        with sqlite3.connect(self.db_path) as conn:
            cursor = conn.cursor()
            cursor.execute(
                'UPDATE sessions SET end_time = ? WHERE id = ?',
                (time.time(), session_id)
            )

    def insert_telemetry(self, session_id: int, data: dict):
        with sqlite3.connect(self.db_path) as conn:
            cursor = conn.cursor()
            cursor.execute('''
                INSERT INTO telemetry (session_id, timestamp_ms, rssi, channel, latency, loss)
                VALUES (?, ?, ?, ?, ?, ?)
            ''', (
                session_id,
                data.get("timestamp_ms", 0),
                data.get("rssi", 0),
                data.get("channel", 0),
                data.get("latency", 0.0),
                data.get("loss", 0.0)
            ))

    def insert_telemetry_batch(self, session_id: int, data_list: list):
        with sqlite3.connect(self.db_path) as conn:
            cursor = conn.cursor()
            cursor.executemany('''
                INSERT INTO telemetry (session_id, timestamp_ms, rssi, channel, latency, loss)
                VALUES (?, ?, ?, ?, ?, ?)
            ''', [
                (session_id, d.get("timestamp_ms", 0), d.get("rssi", 0),
                 d.get("channel", 0), d.get("latency", 0.0), d.get("loss", 0.0))
                for d in data_list
            ])

    def get_session_data(self, session_id: int):
        with sqlite3.connect(self.db_path) as conn:
            cursor = conn.cursor()
            cursor.execute('SELECT timestamp_ms, rssi, channel, latency, loss FROM telemetry WHERE session_id = ? ORDER BY timestamp_ms', (session_id,))
            return cursor.fetchall()

    def export_csv(self, session_id: int, file_path: str):
        data = self.get_session_data(session_id)
        with open(file_path, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(['timestamp_ms', 'rssi', 'channel', 'latency', 'loss'])
            writer.writerows(data)
