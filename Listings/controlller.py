import glob
import json
import os
import re
import sys
import time
from typing import Iterable

import serial


def _detect_port():
    env = os.getenv("ROBOT_PORT")
    if env:
        return env
    # Auto-detect micro:bit on macOS
    candidates = glob.glob("/dev/cu.usbmodem*")
    if candidates:
        return candidates[0]
    # Fallback for Windows
    return "COM15"


PORT = _detect_port()
BAUD = int(os.getenv("ROBOT_BAUD", "115200"))
LLM_MODEL = os.getenv("GEMINI_MODEL", "gemini-2.5-flash")
COMMAND_RE = re.compile(r"^([FBLRSCA])(\d*)$", re.IGNORECASE)

ser = serial.Serial(PORT, BAUD, timeout=0.05)
ser.dtr = True
ser.rts = True

print("Waiting for micro:bit to boot...")
time.sleep(2.5)   # allow reset

ser.reset_input_buffer()
ser.reset_output_buffer()

print("Ready.")


def flush_serial():
    ser.reset_input_buffer()


def _send_bytes_slow(data: bytes, inter_byte_ms: float = 5):
    """Send one byte at a time with a gap so the nRF52833 single-byte
    UART RX register is read by firmware before the next byte arrives."""
    for b in data:
        ser.write(bytes([b]))
        ser.flush()
        time.sleep(inter_byte_ms / 1000.0)


def send_command(cmd):
    print(f"\n>>> SEND: {cmd}")
    _send_bytes_slow((cmd + '\n').encode())


def send_param_command(cmd, value):
    msg = f"{cmd}{value}"
    print(f"\n>>> SEND PARAM: {msg.strip()}")
    _send_bytes_slow((msg + '\n').encode())


def wait_for_done(timeout=20.0):
    start = time.time()
    buffer = ""

    while True:
        if ser.in_waiting:
            data = ser.read(ser.in_waiting).decode(errors='ignore')
            buffer += data

            lines = buffer.split('\n')

            for line in lines[:-1]:
                line = line.strip()
                if line:
                    print("RX:", line)

                    if "DONE" in line:
                        return True

            buffer = lines[-1]

        if time.time() - start > timeout:
            print("TIMEOUT (forcing continue)")
            return False


def normalize_command(raw_command):
    """Return a validated command string accepted by the receiver firmware."""
    if isinstance(raw_command, str):
        token = raw_command.strip().upper()
    elif isinstance(raw_command, dict):
        cmd = str(raw_command.get("cmd", "")).strip().upper()
        value = raw_command.get("value")
        token = cmd if value in (None, "") else f"{cmd}{value}"
    else:
        raise ValueError(f"Unsupported command type: {raw_command!r}")

    match = COMMAND_RE.match(token)
    if not match:
        raise ValueError(f"Invalid command: {token!r}")

    cmd, value = match.groups()
    if cmd in ("L", "R", "S", "A") and value:
        raise ValueError(f"{cmd} does not accept a distance in this firmware")
    if cmd in ("F", "B") and value and int(value) <= 0:
        raise ValueError("Distance must be greater than zero")

    return f"{cmd}{value}"


def is_direct_robot_command(text):
    try:
        normalize_command(text)
        return True
    except ValueError:
        return False


def send_robot_sequence(commands: Iterable[str]):
    for raw_command in commands:
        command = normalize_command(raw_command)
        cmd = command[0]

        if len(command) > 1:
            send_param_command(cmd, int(command[1:]))
        else:
            send_command(cmd)

        if cmd != "S":
            wait_for_done()


def drain_serial(prefix="RX"):
    lines_seen = False
    buffer = ""

    while ser.in_waiting:
        data = ser.read(ser.in_waiting).decode(errors='ignore')
        buffer += data

    for line in buffer.replace("\r", "").split("\n"):
        line = line.strip()
        if line:
            print(f"{prefix}:", line)
            lines_seen = True

    return lines_seen


def monitor_serial():
    print("Monitoring serial. Press Ctrl+C to stop.")
    buffer = ""

    while True:
        if ser.in_waiting:
            data = ser.read(ser.in_waiting).decode(errors='ignore')
            buffer += data.replace("\r", "")

            lines = buffer.split("\n")
            for line in lines[:-1]:
                line = line.strip()
                if line:
                    print("RX:", line)

            buffer = lines[-1]

        time.sleep(0.02)


def prompt_for_manual_commands():
    manual_input = input(
        "Enter manual command(s), for example F20 or F20,L,F20, or press Enter to skip: "
    ).strip()
    if not manual_input:
        return []

    commands = [part for part in re.split(r"[\s,]+", manual_input.upper()) if part]
    return [normalize_command(command) for command in commands]


def plan_with_llm(goal):
    api_key = os.getenv("GEMINI_API_KEY")
    if not api_key:
        raise RuntimeError("GEMINI_API_KEY is not set")

    try:
        from google import genai
    except ImportError as exc:
        raise RuntimeError("Install the Gemini Python package first: pip install google-genai") from exc

    client = genai.Client(api_key=api_key)
    schema = {
        "type": "object",
        "properties": {
            "commands": {
                "type": "array",
                "minItems": 1,
                "maxItems": 12,
                "items": {
                    "type": "object",
                    "properties": {
                        "cmd": {"type": "string", "enum": ["F", "B", "L", "R", "S", "C", "A"]},
                        "value": {"type": "integer", "minimum": 1, "maximum": 100, "nullable": True}
                    },
                    "required": ["cmd", "value"],
                    "propertyOrdering": ["cmd", "value"]
                }
            },
            "notes": {"type": "string"}
        },
        "required": ["commands", "notes"],
        "propertyOrdering": ["commands", "notes"]
    }

    prompt = (
        "Convert this natural-language navigation request into commands for a small "
        "micro:bit robot.\n\n"
        "Available commands:\n"
        "- F with optional distance in centimeters, for example F20\n"
        "- B with optional distance in centimeters, for example B10\n"
        "- L for a fixed 90 degree left turn\n"
        "- R for a fixed 90 degree right turn\n"
        "- C with optional value to move in a circle/arc\n"
        "- A to enter autonomous obstacle avoidance mode\n"
        "- S to stop\n\n"
        "Keep plans short and conservative. Do not use L/R distances because the "
        "firmware ignores turn values.\n\n"
        f"Request: {goal}"
    )
    response = client.models.generate_content(
        model=LLM_MODEL,
        contents=prompt,
        config={
            "response_mime_type": "application/json",
            "response_schema": schema,
        },
    )

    plan = json.loads(response.text)
    commands = [normalize_command(command) for command in plan["commands"]]
    return commands, plan.get("notes", "")


def interactive_loop():
    print(f"Serial: {PORT} @ {BAUD}")
    print(f"LLM model: {LLM_MODEL}")
    print("Enter raw commands like F20/L/R/S, or type a natural-language goal.")
    print("Use Q to quit.")

    while True:
        drain_serial()
        user_input = input("\nCommand or goal: ").strip()

        if user_input.upper() == "Q":
            break

        if not user_input:
            continue

        if is_direct_robot_command(user_input):
            send_robot_sequence([user_input])
            continue

        try:
            commands, notes = plan_with_llm(user_input)
        except Exception as exc:
            print(f"LLM planning failed: {exc}")
            try:
                commands = prompt_for_manual_commands()
            except ValueError as manual_exc:
                print(f"Invalid manual command: {manual_exc}")
                continue

            if commands:
                send_robot_sequence(commands)
            continue

        print("LLM plan:", " -> ".join(commands))
        if notes:
            print("Notes:", notes)

        confirm = input("Execute this plan? [y/N]: ").strip().lower()
        if confirm in ["y", "yes"]:
            send_robot_sequence(commands)
        else:
            print("Plan skipped.")


if __name__ == "__main__":
    try:
        if "--monitor" in sys.argv:
            monitor_serial()
        else:
            interactive_loop()
    except KeyboardInterrupt:
        print("\nStopping.")
        try:
            send_command("S")
        except Exception:
            pass
        sys.exit(0)
