#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
cd "$root"
output=${1:?Usage: sh packaging/build-rpm.sh ABSOLUTE_OUTPUT}
case "$output" in /*) ;; *) echo "Output path must be absolute" >&2; exit 1 ;; esac
. /etc/os-release
test "$ID" = opensuse-leap
test "$VERSION_ID" = 16.0
test "$(uname -m)" = x86_64
tmp=$(mktemp -d)
trap 'rm -rf -- "$tmp"' EXIT HUP INT TERM
mkdir -p "$tmp/source" "$tmp/rpmbuild/SOURCES" "$output"
git archive --prefix=tnt-2.0.0-rc1/ HEAD | tar -C "$tmp/source" -xf -
printf 'Repository: https://github.com/do2bbc/tnt\nSource commit: %s\nTarget: openSUSE Leap 16 x86_64\n' \
  "$(git rev-parse HEAD)" > "$tmp/source/tnt-2.0.0-rc1/BUILD.txt"
tar -C "$tmp/source" -czf "$tmp/rpmbuild/SOURCES/tnt-2.0.0-rc1.tar.gz" tnt-2.0.0-rc1
rpmbuild -ba --define "_topdir $tmp/rpmbuild" tnt.spec
cp "$tmp/rpmbuild/RPMS/x86_64/tnt-2.0.0~rc1-1.x86_64.rpm" "$output/tnt-2.0.0-rc1-1.x86_64.rpm"
cp "$tmp/rpmbuild/SRPMS/tnt-2.0.0~rc1-1.src.rpm" "$output/tnt-2.0.0-rc1-1.src.rpm"
cp "$tmp/rpmbuild/SOURCES/tnt-2.0.0-rc1.tar.gz" "$output/"
