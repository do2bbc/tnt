import os
from pathlib import Path
import subprocess
import sys
import tempfile


def main(package, arch):
    with tempfile.TemporaryDirectory() as temporary:
        root = Path(temporary)
        if package.endswith(".deb"):
            subprocess.run(["dpkg-deb", "-x", package, root], check=True)
            subprocess.run(["dpkg-deb", "-e", package, root / "control"], check=True)
            subprocess.run(["md5sum", "-c", root / "control/md5sums"], cwd=root,
                           check=True, stdout=subprocess.DEVNULL)
            assert not any((root / "control" / script).exists()
                           for script in ("preinst", "postinst", "prerm", "postrm"))
            program = root / "usr/bin/tnt"
            prefix = {
                "amd64": [],
                "arm64": ["qemu-aarch64", "-L", "/"],
                "armhf": ["qemu-arm", "-L", "/"],
            }[arch]
            for name in ("tnt", "tntc"):
                result = subprocess.run(
                    [*prefix, str(root / "usr/bin" / name), "--invalid-option"],
                    stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=15)
                assert result.returncode != 0
                assert b"Usage" in result.stdout, result.stdout
            assert not (root / "etc/tnt/tnt.ini").exists()
            assert (root / "etc/tnt/tnt.ini.ex").exists()
            assert (root / "usr/share/doc/tnt/SOURCE.tar.gz").exists()
        else:
            result = subprocess.run(["rpm", "-qp", "--scripts", package],
                                    check=True, stdout=subprocess.PIPE)
            assert not result.stdout.strip(), result.stdout
            subprocess.run(["rpm", "-U", "--replacepkgs", package], check=True)
            result = subprocess.run(["/usr/bin/tnt", "--invalid-option"],
                                    stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                    timeout=15)
            assert result.returncode != 0 and b"Usage" in result.stdout, result.stdout
            assert not Path("/etc/tnt/tnt.ini").exists()
            assert Path("/etc/tnt/tnt.ini.ex").exists()
        print(f"{arch}: packaged programs load correctly; no service or active configuration")


if __name__ == "__main__":
    main(str(Path(sys.argv[1]).resolve()), sys.argv[2])
