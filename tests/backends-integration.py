import errno
import fcntl
import os
from pathlib import Path
import pty
import select
import shutil
import signal
import socket
import struct
import subprocess
import sys
import tempfile
import termios
import threading
import time


def run(source, build, disconnect=False, scrollback_wakeup=False,
        backend_timeout=False, partial_response=False, stalled=False, polling=False,
        finish_waiting="resume"):
    with tempfile.TemporaryDirectory(prefix="tnt-backends-") as directory:
        root = Path(directory)
        requests = [[], []]
        failures = []
        stop = threading.Event()
        ready = [threading.Event(), threading.Event()]
        disconnected = threading.Event()
        delayed_response = threading.Event()
        waiting = threading.Event()
        release_response = threading.Event()
        resumed = threading.Event()
        calls = ["CBTEST", "HAMTEST"]

        def backend(index):
            try:
                with socket.socket(socket.AF_UNIX) as server:
                    server.bind(str(root / f"backend{index}"))
                    server.listen(1)
                    server.settimeout(0.1)
                    ready[index].set()
                    while not stop.is_set():
                        try:
                            connection, _ = server.accept()
                            break
                        except socket.timeout:
                            continue
                    else:
                        return
                    with connection:
                        connection.settimeout(0.1)
                        pending = bytearray()
                        hostmode = False
                        delay_next_response = False
                        while not stop.is_set():
                            if index == 1 and disconnected.is_set():
                                return
                            try:
                                part = connection.recv(4096)
                            except socket.timeout:
                                continue
                            if not part:
                                return
                            pending.extend(part)
                            if not hostmode:
                                marker = pending.find(b"JHOST 1\r")
                                if marker < 0:
                                    continue
                                del pending[: marker + len(b"JHOST 1\r")]
                                hostmode = True
                            while len(pending) >= 3 and len(pending) >= pending[2] + 4:
                                channel, code, size = pending[:3]
                                command = bytes(pending[3 : size + 4])
                                del pending[: size + 4]
                                requests[index].append((channel, code, command))
                                assert channel <= 10, (index, channel, command)
                                if command == b"JHOST 0":
                                    return
                                if (scrollback_wakeup or backend_timeout) and command == b"CCBLAST":
                                    text = b"scrollback timeout regression\r"
                                    connection.sendall(bytes((channel, 7, len(text) - 1)) + text)
                                    delay_next_response = True
                                    continue
                                if command == b"L":
                                    text = b"0 0" if channel == 0 else b"0 0 0 0 0 0"
                                elif command == b"I":
                                    text = calls[index].encode()
                                elif command == b"C":
                                    text = b"CHANNEL NOT CONNECTED"
                                elif command == b"@B":
                                    text = b"5000"
                                else:
                                    text = None
                                response = bytes((channel, 0)) if text is None else (
                                    bytes((channel, 1)) + text + b"\0"
                                )
                                if delay_next_response and (not backend_timeout or command == b"@B"):
                                    delay_next_response = False
                                    if partial_response:
                                        connection.sendall(response[:3])
                                        response = response[3:]
                                    before_wait = [len(items) for items in requests]
                                    waiting.set()
                                    if not release_response.wait(
                                        60 if stalled else 9 if backend_timeout else 2.5
                                    ):
                                        if stalled:
                                            raise AssertionError("Stalled backend was not released")
                                    if stop.is_set():
                                        return
                                    assert [len(items) for items in requests] == before_wait, (
                                        "TNT resent or switched backend while awaiting a response", requests
                                    )
                                    assert not select.select([connection], [], [], 0)[0], (
                                        "Additional traffic was queued on the stalled backend"
                                    )
                                    if finish_waiting == "disconnect":
                                        return
                                    if finish_waiting == "malformed":
                                        response = b"\xff"
                                    delayed_response.set()
                                elif delayed_response.is_set():
                                    resumed.set()
                                # Exercise channel/code and text split across reads.
                                connection.sendall(response[:1])
                                time.sleep(0.001)
                                connection.sendall(response[1:])
            except BaseException as error:
                if not stop.is_set():
                    failures.append(error)
                    ready[index].set()

        for path in (source / "examples").iterdir():
            if path.is_file():
                shutil.copyfile(path, root / path.name)
        for path in ("newmail", "bcast/save", "bcast/newmail"):
            (root / path).mkdir(parents=True, exist_ok=True)
        ini = (build / "examples/tnt.ini").read_text()
        lines = []
        for line in ini.splitlines():
            parts = line.split()
            if parts and not line.startswith("#"):
                key = parts[0]
                if key.endswith("_dir"):
                    line = f"{key} {root}"
                elif key.startswith("lines_"):
                    line = f"{key} 30"
                elif key == "soft_tnc":
                    line = "soft_tnc 1"
                elif key == "device":
                    line = f"device {root}/backend0"
                elif key == "tnt_lockfile":
                    line = f"tnt_lockfile {root}/lock"
                elif key == "tnt_errlog":
                    line = f"tnt_errlog {root}/errors"
                elif key == "use_select":
                    line = f"use_select {0 if polling else 1}"
            lines.append(line)
        lines.extend([f"backend2_device {root}/backend1", "backend2_channels 10"])
        lines.append(f"tnt_errlog {root}/errors")
        (root / "tnt.ini").write_text("\n".join(lines) + "\n")
        (root / "tnt.up").write_text(
            "backend 2\nm a\nchannel 0\nmycall\nchannel 11\nc HAMDEST\n"
            "channel 20\nc HAMLAST\nchannel 1\nc CBDEST\n"
            "backend 1\nm n\nchannel 10\nc CBLAST\n"
        )
        (root / "tnt.dwn").write_text("")
        workers = [threading.Thread(target=backend, args=(i,)) for i in range(2)]
        for worker in workers:
            worker.start()
        process = None
        master, slave = pty.openpty()
        fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack("HHHH", 24, 100, 0, 0))
        transcript = bytearray()
        try:
            assert all(event.wait(5) for event in ready), "Mock backends failed to start"
            assert not failures, failures
            process = subprocess.Popen(
                [str(build / "src/tnt"), "-i", str(root / "tnt.ini")],
                stdin=slave, stdout=slave, stderr=slave, cwd=root,
                env={**os.environ, "TERM": "xterm", "LC_ALL": "C"},
            )
            os.close(slave)
            slave = -1
            deadline = time.monotonic() + 20
            while time.monotonic() < deadline and process.poll() is None:
                if select.select([master], [], [], 0.1)[0]:
                    try:
                        transcript.extend(os.read(master, 65536))
                    except OSError as error:
                        if error.errno != errno.EIO:
                            raise
                        break
                if any(command == b"CCBLAST" for _, _, command in requests[0]):
                    break
            assert process.poll() is None, transcript.decode("latin1")
            for index in range(2):
                polled = {channel for channel, _, command in requests[index] if command == b"L"}
                assert set(range(1, 11)) <= polled, (index, polled)
            assert (1, 1, b"CHAMDEST") in requests[1], requests
            assert (10, 1, b"CHAMLAST") in requests[1], requests
            assert (1, 1, b"CCBDEST") in requests[0], requests
            assert (10, 1, b"CCBLAST") in requests[0], requests
            assert (0, 1, b"Ma") in requests[1], requests
            assert (0, 1, b"I") in requests[1], requests
            assert (0, 1, b"Mn") in requests[0], requests
            assert all(b"HAM" not in command for _, _, command in requests[0]), requests
            assert all(b"CB" not in command for _, _, command in requests[1]), requests
            if not (disconnect or scrollback_wakeup or backend_timeout):
                def wait_command(index, command):
                    deadline = time.monotonic() + 5
                    while time.monotonic() < deadline:
                        if (1, 1, command) in requests[index]:
                            return
                        if select.select([master], [], [], 0.1)[0]:
                            transcript.extend(os.read(master, 65536))
                    raise AssertionError((index, command, requests))

                # The command view keeps pending input while switching backend.
                os.write(master, b"c MOUSE")
                os.write(master, b"\033[<0;81;24M")
                os.write(master, b"HAM\r")
                wait_command(1, b"CMOUSEHAM")
                os.write(master, b"\033[<64;12;5M\033[<65;12;5M")
                os.write(master, b"\033[<0;81;24M")
                os.write(master, b"c MOUSECB\r")
                wait_command(0, b"CMOUSECB")
            if scrollback_wakeup:
                assert delayed_response.wait(5), "Delayed response did not complete"
                assert process.poll() is None, "Scrollback wakeup stopped TNT"
            if backend_timeout:
                deadline = time.monotonic() + (18 if stalled else 14)
                warning_count = 2 if stalled else 1
                while time.monotonic() < deadline and process.poll() is None:
                    if select.select([master], [], [], 0.1)[0]:
                        try:
                            transcript.extend(os.read(master, 65536))
                        except OSError as error:
                            if error.errno != errno.EIO:
                                raise
                            break
                    errors = (root / "errors").read_text()
                    if errors.count("still waiting") >= warning_count:
                        if finish_waiting == "shutdown":
                            break
                        if stalled:
                            release_response.set()
                        if resumed.is_set():
                            break
                assert waiting.is_set(), "The @B delay was not exercised"
                if finish_waiting in ("resume", "shutdown"):
                    assert process.poll() is None, "A temporary response delay terminated TNT"
                if finish_waiting == "resume":
                    assert resumed.wait(2), "Polling did not resume after the delayed response"
                errors = (root / "errors").read_text()
                assert errors.count("still waiting") >= warning_count, errors
                assert "backend CB" in errors and "request 6" in errors, errors
            if disconnect:
                disconnected.set()
            elif finish_waiting in ("resume", "shutdown"):
                process.send_signal(signal.SIGTERM)
            deadline = time.monotonic() + 8
            while time.monotonic() < deadline and process.poll() is None:
                if select.select([master], [], [], 0.1)[0]:
                    try:
                        transcript.extend(os.read(master, 65536))
                    except OSError as error:
                        if error.errno != errno.EIO:
                            raise
            result = process.wait(timeout=1)
            failed = disconnect or finish_waiting in ("disconnect", "malformed")
            assert (result > 0 if failed else result == 0), result
            if disconnect:
                errors = (root / "errors").read_text()
                assert ("backend disconnected" in errors or
                        "backend HAM write failed" in errors), errors
            if finish_waiting == "disconnect":
                assert "backend disconnected" in (root / "errors").read_text()
            if finish_waiting == "malformed":
                assert "hostmode protocol error" in (root / "errors").read_text()
            assert not failures, failures
            print("Two backends: 10+10 channels, command isolation, monitor selection and clean exit")
        except BaseException:
            print(transcript[-8000:].decode("latin1"), file=sys.stderr)
            print("Mock failures:", failures, file=sys.stderr)
            print("Latest requests:", [items[-15:] for items in requests], file=sys.stderr)
            errors = root / "errors"
            if errors.exists():
                print(errors.read_text(), file=sys.stderr)
            raise
        finally:
            stop.set()
            release_response.set()
            if process is not None and process.poll() is None:
                process.kill()
                process.wait(timeout=5)
            if slave >= 0:
                os.close(slave)
            os.close(master)
            for worker in workers:
                worker.join(timeout=16)


if __name__ == "__main__":
    run(Path(sys.argv[1]).resolve(), Path(sys.argv[2]).resolve(),
        disconnect="--disconnect" in sys.argv[3:],
        scrollback_wakeup="--scrollback-wakeup" in sys.argv[3:],
        backend_timeout="--backend-timeout" in sys.argv[3:],
        partial_response="--partial-response" in sys.argv[3:],
        stalled="--stalled" in sys.argv[3:],
        polling="--polling" in sys.argv[3:],
        finish_waiting=next(
            (mode for mode in ("shutdown", "disconnect", "malformed")
             if f"--{mode}-waiting" in sys.argv[3:]), "resume"
        ))
