#!/bin/bash
# Build the AV .deb from an already-configured meson build directory.
#
# The package used to be assembled by hand, which is how a .deb whose filename
# said "av" ended up shipping /usr/bin/medea: the pkgroot was left over from a
# build predating the rename. Everything here is derived from the build tree on
# each run instead, so that cannot happen again.
#
# Usage:  extras/av/build-deb.sh [builddir]
#
# Env:
#   AV_DEB_VERSION   override the version (default: derived from git describe)
#   AV_DEB_OUTDIR    where to write the .deb (default: debbuild/)
set -euo pipefail

REPO_ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
cd "$REPO_ROOT"

BUILD_DIR=${1:-build}
OUTDIR=${AV_DEB_OUTDIR:-debbuild}
PKGROOT="$OUTDIR/av_pkgroot"

if [ ! -f "$BUILD_DIR/build.ninja" ]; then
    echo "error: '$BUILD_DIR' is not a configured meson build directory." >&2
    echo "       run extras/av/configure-av.sh $BUILD_DIR first." >&2
    exit 1
fi

# Debian versions may not contain '-' in the upstream part unless a revision
# follows, and `git describe` is full of them. 4.0.0-dev-38351-gcc8c4e7ece
# becomes 4.0.0~dev.38351.gcc8c4e7ece-1; the '~' sorts *below* a future plain
# 4.0.0, which is what we want for a -dev build.
if [ -n "${AV_DEB_VERSION:-}" ]; then
    VERSION="$AV_DEB_VERSION"
else
    DESC=$(git describe --always HEAD)
    VERSION="$(printf '%s' "$DESC" | sed -e 's/-dev-/~dev./' -e 's/-/./g')-1"
fi
ARCH=$(dpkg --print-architecture)

echo "==> version $VERSION ($ARCH)"

# Always start from an empty pkgroot. Reusing one is exactly how stale paths
# from a previous naming survive into a new package.
rm -rf "$PKGROOT"
mkdir -p "$PKGROOT"

echo "==> installing into pkgroot"
DESTDIR="$REPO_ROOT/$PKGROOT" ninja -C "$BUILD_DIR" install >/dev/null

echo "==> pruning development files"
find "$PKGROOT/usr/lib" -name '*.la' -delete 2>/dev/null || true
find "$PKGROOT/usr/lib" -path '*/cmake/*' -delete 2>/dev/null || true
find "$PKGROOT/usr/lib" -path '*/pkgconfig/*' -delete 2>/dev/null || true
rm -rf "$PKGROOT/usr/include" "$PKGROOT/usr/share/doc" "$PKGROOT/usr/share/man"

# Fail loudly rather than shipping a package that installs the wrong binary.
if [ ! -x "$PKGROOT/usr/bin/av" ]; then
    echo "error: $PKGROOT/usr/bin/av is missing - the build tree is stale or" >&2
    echo "       was configured under a different name. Reconfigure and rebuild." >&2
    exit 1
fi
if find "$PKGROOT" -iname '*medea*' -print -quit | grep -q .; then
    echo "error: pre-rename 'medea' paths present in the pkgroot:" >&2
    find "$PKGROOT" -iname '*medea*' >&2
    exit 1
fi

echo "==> generating control"
mkdir -p "$PKGROOT/DEBIAN"
INSTALLED_SIZE=$(du -sk "$PKGROOT/usr" | cut -f1)
sed -e "s|@VERSION@|$VERSION|" \
    -e "s|@ARCH@|$ARCH|" \
    -e "s|@INSTALLED_SIZE@|$INSTALLED_SIZE|" \
    extras/av/deb/control.in > "$PKGROOT/DEBIAN/control"
install -m 755 extras/av/deb/postinst extras/av/deb/postrm "$PKGROOT/DEBIAN/"

# --root-owner-group gives every file root:root without needing fakeroot or
# sudo; without it the package inherits the building user's uid/gid.
echo "==> packaging"
DEB="$OUTDIR/av_${VERSION}_${ARCH}.deb"
dpkg-deb --root-owner-group --build "$PKGROOT" "$DEB" >/dev/null

echo
echo "built: $DEB"
ls -lh "$DEB" | awk '{print "  size:   " $5}'
echo "  sha256: $(sha256sum "$DEB" | cut -d' ' -f1)"
echo "  binary: $(dpkg-deb -c "$DEB" | awk '/usr\/bin\/av$/ {print $6}')"
