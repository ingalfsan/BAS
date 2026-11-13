#!/usr/bin/env python3
"""Compile and run the Arduino-independent thermal core tests."""

from pathlib import Path
import shutil
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[1]
SKETCH = ROOT / "firmware" / "BASPrototype"
TEST_SOURCE = ROOT / "tests" / "test_arduino_thermal.cpp"


def main() -> int:
    compiler = next(
        (shutil.which(name) for name in ("g++", "clang++") if shutil.which(name)),
        None,
    )
    if compiler is None:
        print("No host C++ compiler found. Install GCC or Clang to run these tests.",
              file=sys.stderr)
        return 2

    with tempfile.TemporaryDirectory(prefix="bas-arduino-thermal-tests-") as temp_dir:
        executable = Path(temp_dir) / "test_arduino_thermal"
        command = [
            compiler,
            "-std=c++11",
            "-Wall",
            "-Wextra",
            "-Werror",
            "-pedantic",
            "-I",
            str(SKETCH),
            str(TEST_SOURCE),
            str(SKETCH / "ThermalCore.cpp"),
            "-lm",
            "-o",
            str(executable),
        ]
        subprocess.run(command, check=True)
        subprocess.run([str(executable)], check=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
