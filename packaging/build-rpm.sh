#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
cd "$root"
output=${1:?Usage: sh packaging/build-rpm.sh ABSOLUTE_OUTPUT}
sound=${2:-on}
case "$output" in /*) ;; *) echo "Output path must be absolute" >&2; exit 1 ;; esac
case "$sound" in
  on) cmake_sound=ON; suffix= ;;
  off) cmake_sound=OFF; suffix=-nosound ;;
  *) echo "Sound option must be on or off" >&2; exit 1 ;;
esac
. /etc/os-release
test "$ID" = opensuse-leap
test "$VERSION_ID" = 16.0
test "$(uname -m)" = x86_64
version=$(sed -n 's/^set(TNT_VERSION "\(.*\)")/\1/p' CMakeLists.txt)
test -n "$version"
rpm_version=$(printf '%s' "$version" | sed 's/-rc/~rc/')
rpm_release=1
tmp=$(mktemp -d)
trap 'rm -rf -- "$tmp"' EXIT HUP INT TERM
mkdir -p "$tmp/source" "$tmp/rpmbuild/SOURCES" "$output"
git archive --prefix="tnt-$version/" HEAD | tar -C "$tmp/source" -xf -
printf 'Repository: https://github.com/do2bbc/tnt\nSource commit: %s\nTarget: openSUSE Leap 16 x86_64\nSound: %s\n' \
  "$(git rev-parse HEAD)" "$sound" > "$tmp/source/tnt-$version/BUILD.txt"
tar -C "$tmp/source" -czf "$tmp/rpmbuild/SOURCES/tnt-$version.tar.gz" "tnt-$version"
rpmbuild -ba --define "_topdir $tmp/rpmbuild" \
  --define "tnt_version $rpm_version" --define "tnt_source_version $version" \
  --define "tnt_release $rpm_release" \
  --define "tnt_sound $cmake_sound" tnt.spec
cp "$tmp/rpmbuild/RPMS/x86_64/tnt-$rpm_version-$rpm_release.x86_64.rpm" \
  "$output/tnt-$version-$rpm_release${suffix}.x86_64.rpm"
if [ "$sound" = on ]; then
  cp "$tmp/rpmbuild/SRPMS/tnt-$rpm_version-$rpm_release.src.rpm" "$output/"
  cp "$tmp/rpmbuild/SOURCES/tnt-$version.tar.gz" "$output/"
fi
