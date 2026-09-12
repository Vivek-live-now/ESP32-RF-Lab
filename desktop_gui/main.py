import os
import sys

# Ensure rflab module is in path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from rflab.ui.main_window import run

if __name__ == "__main__":
    run()
