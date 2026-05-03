import argparse
import json
import os
import sys
import time


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", required=True, help="Serial port (e.g. COM5)")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--out", default="debug-b979ad.log")
    args = ap.parse_args()

    try:
        import serial  # type: ignore
    except Exception:
        sys.stderr.write("pyserial not installed. Run: pip install pyserial\n")
        return 2

    out_path = os.path.abspath(args.out)
    ser = serial.Serial(args.port, args.baud, timeout=0.2)
    sys.stderr.write(f"Capturing from {args.port} @ {args.baud} to {out_path}\n")

    with open(out_path, "a", encoding="utf-8") as f:
        while True:
            try:
                line = ser.readline()
                if not line:
                    continue
                try:
                    s = line.decode("utf-8", errors="ignore").strip()
                except Exception:
                    continue
                if not s:
                    continue
                if s.startswith("{") and "sessionId" in s and "b979ad" in s:
                    # validate JSON
                    try:
                        obj = json.loads(s)
                    except Exception:
                        continue
                    f.write(json.dumps(obj) + "\n")
                    f.flush()
            except KeyboardInterrupt:
                sys.stderr.write("Stopped.\n")
                return 0
            except Exception:
                time.sleep(0.1)


if __name__ == "__main__":
    raise SystemExit(main())

