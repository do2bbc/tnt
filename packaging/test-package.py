import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile


def main(package, arch, sound):
    os.environ["TERM"] = "xterm"
    os.environ["LC_ALL"] = "C"
    with tempfile.TemporaryDirectory() as temporary:
        root = Path(temporary)
        if package.endswith(".deb"):
            metadata = subprocess.check_output(
                ["dpkg-deb", "-f", package, "Package", "Version", "Architecture"])
            assert b"Package: tnt\n" in metadata, metadata
            assert b"Version: 2.0.0-1\n" in metadata, metadata
            assert f"Architecture: {arch}\n".encode() in metadata, metadata
            subprocess.run(["dpkg-deb", "-x", package, root], check=True)
            subprocess.run(["dpkg-deb", "-e", package, root / "control"], check=True)
            subprocess.run(["md5sum", "-c", root / "control/md5sums"], cwd=root,
                           check=True, stdout=subprocess.DEVNULL)
            assert not any((root / "control" / script).exists()
                           for script in ("preinst", "postinst", "prerm", "postrm"))
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
            binary = (root / "usr/bin/tnt").read_bytes()
        else:
            metadata = subprocess.check_output(
                ["rpm", "-qp", "--qf", "%{NAME} %{VERSION}-%{RELEASE} %{ARCH}", package])
            assert metadata == b"tnt 2.0.0-1 x86_64", metadata
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
            binary = Path("/usr/bin/tnt").read_bytes()
        assert re.search(rb"V2\.0\.0(?:[ (])", binary)
        has_sound = bool(re.search(rb"TNT V2\.0\.0\([^)]*Sound\)", binary))
        assert has_sound == (sound == "on"), (sound, has_sound)
        print(f"{arch}: packaged programs load correctly; no service or active configuration")


def preservation(package):
    active = Path("/etc/tnt/tnt.ini")
    example = Path("/etc/tnt/tnt.ini.ex")
    private = Path("/usr/local/bin/tnt")
    assert not active.exists() and not private.exists()
    if package.endswith(".deb"):
        install = ["apt-get", "-y",
                   "-o", "Dpkg::Options::=--path-include=/usr/share/doc/tnt/*",
                   "install", package]
        reinstall = ["dpkg", "--force-confold", "-i", package]
        remove = ["apt-get", "-y", "remove", "tnt"]
    else:
        install = ["zypper", "--non-interactive", "--no-gpg-checks",
                   "install", "--no-recommends", package]
        reinstall = ["rpm", "-U", "--replacepkgs", package]
        remove = ["rpm", "-e", "tnt"]
    subprocess.run(install, check=True)
    assert not active.exists() and not private.exists()
    active.write_text("active configuration sentinel\n")
    private.parent.mkdir(parents=True, exist_ok=True)
    private.write_text("private executable sentinel\n")
    example.write_text("modified example sentinel\n")
    subprocess.run(reinstall, check=True)
    assert active.read_text() == "active configuration sentinel\n"
    assert private.read_text() == "private executable sentinel\n"
    assert example.read_text() == "modified example sentinel\n"
    subprocess.run(remove, check=True)
    assert active.read_text() == "active configuration sentinel\n"
    assert private.read_text() == "private executable sentinel\n"
    assert not Path("/usr/bin/tnt").exists()
    print("Install/reinstall/removal preserve active configuration and private binary")


def switch(sound_package, silent_package):
    active = Path("/etc/tnt/tnt.ini")
    example = Path("/etc/tnt/tnt.ini.ex")
    assert not active.exists()
    if sound_package.endswith(".deb"):
        install = ["apt-get", "-y", "install", sound_package]
        replace = ["apt-get", "-y", "-o", "Dpkg::Options::=--force-confold",
                   "--reinstall", "install"]
    else:
        install = ["zypper", "--non-interactive", "--no-gpg-checks",
                   "install", "--no-recommends", sound_package]
        replace = ["rpm", "-U", "--replacepkgs"]
    subprocess.run(install, check=True)
    active.write_text("active configuration sentinel\n")
    example.write_text("modified example sentinel\n")
    for package, sound in ((silent_package, False), (sound_package, True)):
        subprocess.run([*replace, package], check=True)
        binary = Path("/usr/bin/tnt").read_bytes()
        assert bool(re.search(rb"TNT V2\.0\.0\([^)]*Sound\)", binary)) == sound
        assert active.read_text() == "active configuration sentinel\n"
        assert example.read_text() == "modified example sentinel\n"
    print("Both variant switches preserve the active and modified configuration")


if __name__ == "__main__":
    assert len(sys.argv) in (3, 4), "Usage: test-package.py PACKAGE ARCH|--preservation [on|off]"
    if sys.argv[1] == "--switch":
        assert len(sys.argv) == 4, "Usage: test-package.py --switch SOUND_PACKAGE SILENT_PACKAGE"
        switch(*(str(Path(path).resolve()) for path in sys.argv[2:]))
        sys.exit(0)
    if sys.argv[2] == "--preservation":
        preservation(str(Path(sys.argv[1]).resolve()))
        sys.exit(0)
    sound = sys.argv[3] if len(sys.argv) == 4 else "on"
    assert sound in ("on", "off"), sound
    main(str(Path(sys.argv[1]).resolve()), sys.argv[2], sound)
