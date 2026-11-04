#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
cd "$root"
arch=${1:?Usage: sh packaging/build-deb.sh amd64|arm64|armhf ABSOLUTE_OUTPUT}
output=${2:?Missing absolute output directory}
case "$output" in /*) ;; *) echo "Output path must be absolute" >&2; exit 1 ;; esac
case "$arch" in
  amd64) host=x86_64-linux-gnu; machine='Advanced Micro Devices X86-64' ;;
  arm64) host=aarch64-linux-gnu; machine=AArch64 ;;
  armhf) host=arm-linux-gnueabihf; machine=ARM ;;
  *) echo "Unsupported architecture: $arch" >&2; exit 1 ;;
esac
test "$(getconf GNU_LIBC_VERSION)" = 'glibc 2.36'
test "$(dpkg --print-architecture)" = amd64
version=$(sed -n 's/^AC_INIT(\[tnt\], \[\(.*\)\])/\1/p' configure.ac)
debversion=$(printf '%s' "$version" | sed 's/-rc/~rc/')-1
tmp=$(mktemp -d)
trap 'rm -rf -- "$tmp"' EXIT HUP INT TERM
mkdir -p "$tmp/source" "$tmp/debian/tnt" "$output"
git archive HEAD | tar -C "$tmp/source" -xf -
source="$tmp/source"
package="$tmp/debian/tnt"
export SOURCE_DATE_EPOCH=$(git show -s --format=%ct HEAD)
cd "$source"
autoreconf -fi
export PKG_CONFIG_LIBDIR="/usr/lib/$host/pkgconfig:/usr/share/pkgconfig"
./configure --prefix=/usr --sysconfdir=/etc --localstatedir=/var \
  --host="$host" --build=x86_64-linux-gnu CC="$host-gcc"
make -j2
if [ "$arch" = amd64 ]; then
  make check
fi
make DESTDIR="$package" install
doc="$package/usr/share/doc/tnt"
mkdir -p "$package/DEBIAN" "$doc"
cp COPYING "$doc/copyright"
cp NEWS BUILD.md packaging/README.packages "$doc/"
printf 'Repository: https://github.com/do2bbc/tnt\nSource commit: %s\nTarget: Debian 12 %s\n' \
  "$(git -C "$root" rev-parse HEAD)" "$arch" > "$doc/BUILD.txt"
git -C "$root" archive --prefix="tnt-$version/" HEAD | gzip -n > "$doc/SOURCE.tar.gz"
for binary in "$package/usr/bin/tnt" "$package/usr/bin/tntc" \
              "$package/usr/libexec/tnt/help" "$package/usr/libexec/tnt/logcall" \
              "$package/usr/libexec/tnt/tnt-info"; do
  LC_ALL=C readelf -h "$binary" | grep -F 'Machine:' | grep -Fq "$machine"
done
if [ "$arch" = armhf ]; then
  readelf -A "$package/usr/bin/tnt" | grep -q 'Tag_CPU_arch: v7'
  readelf -A "$package/usr/bin/tnt" | grep -q 'Tag_ABI_VFP_args: VFP registers'
fi
printf 'Source: tnt\n\nPackage: tnt\nArchitecture: any\n' > "$tmp/debian/control"
dependencies=$(cd "$tmp" && DEB_HOST_ARCH="$arch" dpkg-shlibdeps -O \
  -l"/usr/lib/$host" -edebian/tnt/usr/bin/tnt -edebian/tnt/usr/bin/tntc \
  -edebian/tnt/usr/libexec/tnt/help -edebian/tnt/usr/libexec/tnt/logcall \
  -edebian/tnt/usr/libexec/tnt/tnt-info \
  2>"$tmp/dependencies.log" | sed 's/^shlibs:Depends=//')
cat "$tmp/dependencies.log" >&2
test -n "$dependencies"
cat > "$package/DEBIAN/control" <<EOF
Package: tnt
Version: $debversion
Architecture: $arch
Maintainer: TNT contributors <tnt@users.noreply.github.com>
Section: hamradio
Priority: optional
Depends: $dependencies
Recommends: dialog
Installed-Size: $(du -sk "$package/usr" | cut -f1)
Homepage: https://github.com/do2bbc/tnt
Description: Packet-radio hostmode terminal with modern Linux terminal support
 Release candidate with UTF-8 terminal boundaries, resizing and mouse channels.
 No service or active user configuration is installed automatically.
EOF
cd "$package"
find etc -type f | LC_ALL=C sort | sed 's@^@/@' > DEBIAN/conffiles
find usr etc -type f -print0 | LC_ALL=C sort -z | xargs -0 md5sum > DEBIAN/md5sums
dpkg-deb --root-owner-group --build "$package" "$output/tnt_${version}-1_${arch}.deb"
