#!/bin/sh
# Configure AV: a local-only audio/video player.
#
# Two layers keep the network out. This is the first: every module that opens a
# socket, serves, discovers, streams or casts is left out of the build. The
# second is the snap shipping without the `network` plug, which is enforced by
# snapd in the kernel and cannot be bypassed by a bug in a demuxer. Neither
# layer is sufficient alone; together they are.
#
# Usage:  extras/av/configure-av.sh [builddir] [extra meson args...]
set -e

BUILDDIR=${1:-build}
[ $# -gt 0 ] && shift

SRCDIR=$(cd "$(dirname "$0")/../.." && pwd)
CONTRIB="$SRCDIR/contrib/x86_64-linux-gnu"

# ---------------------------------------------------------------- networking
# Anything that talks to a network, or exists to serve/announce media on one.
NET="
-Dstream_outputs=false
-Dchromecast=disabled
-Dsrt=disabled
-Drist=disabled
-Dlive555=disabled
-Ddsm=disabled
-Dnfs=disabled
-Dsftp=disabled
-Dlibssh2=disabled
-Dupnp=disabled
-Dupnp_server=disabled
-Dmicrodns=disabled
-Davahi=disabled
-Dgnutls=disabled
-Dshout=disabled
-Dfreerdp=disabled
-Dvnc=disabled
-Dsam3=disabled
-Dlibsecret=disabled
"

# ------------------------------------------------------- remote code / update
# The addon manager and updater both fetch and execute remote content. Lua is
# a scripting engine reachable from playlist files, and is a recurring source
# of VLC advisories; a player does not need it.
REMOTE="
-Daddon_manager=false
-Dupdate-check=disabled
-Dlua=disabled
-Dvideolan_manager=false
"

# ------------------------------------------------------------------- encoders
# AV plays; it does not encode or transcode. Dropping the encoders removes a
# large amount of attack surface that no playback path can reach.
ENCODERS="
-Dx264=disabled
-Dx265=disabled
-Dx262=disabled
-Dfdk-aac=disabled
-Dtwolame=disabled
-Dshine=disabled
-Drav1e=disabled
-Dtheoraenc=disabled
-Ddaalaenc=disabled
-Dopenapv=disabled
-Dvpl=disabled
"

# --------------------------------------------------------- capture / hardware
# Capture devices, broadcast tuners and professional SDI hardware. A desktop
# player opens files; it does not need a camera, a DVB tuner or a capture card.
CAPTURE="
-Dscreen=disabled
-Ddc1394=disabled
-Ddv1394=disabled
-Dlinsys=disabled
-Ddecklink=disabled
-Dasdcplib=disabled
-Dopencv=disabled
-Dmtp=disabled
-Dlibdvbpsi=disabled
-Ddvbcsa=disabled
-Daribb24=disabled
-Daribb25=disabled
-Daribcaption=disabled
-Daribsub=disabled
-Dtelx=disabled
-Dzvbi=disabled
"

# ----------------------------------------------------------------- other UIs
# AV has exactly one interface. skins2 in particular parses untrusted XML
# skin archives.
UIS="
-Dskins2=disabled
-Dncurses=disabled
-Dminimal_macosx=disabled
-Dqt_gtk=disabled
-Dqt_qml_debug=false
"

# -------------------------------------------------------- exotic / novelty
# Chiptune and tracker formats, MIDI synthesis, music visualisers and the
# ASCII-art video output. None of these belong in a video and audio player,
# and each is an extra parser reached straight from file input.
MISC="
-Dsid=disabled
-Dgme=disabled
-Dlibmodplug=disabled
-Dfluidsynth=disabled
-Dprojectm=disabled
-Dvsxu=disabled
-Dgoom2=disabled
-Dcaca=disabled
-Dkate=disabled
-Dtiger=disabled
-Dbpg=disabled
-Ddaaladec=disabled
-Dfaad=disabled
-Dlibchromaprint=disabled
"

# ----------------------------------------------------------------- x86-64 only
# AV targets 64-bit Intel/AMD desktops only, so the SIMD paths are switched
# on unconditionally rather than probed.
#
# x86-64-v3 means AVX2 + BMI2 + FMA, i.e. Haswell (2013) and newer. A 10th-gen
# Intel laptop is well inside that. Set AV_MARCH=x86-64-v2 for pre-2013
# hardware, or AV_MARCH=native to tune for this exact machine (in which case
# the build is not portable to another CPU).
MARCH=${AV_MARCH:-x86-64-v3}

ARCH="
-Dsse=enabled
-Davx=enabled
"

# branch_protection is AArch64-only and is left off deliberately; on x86-64 the
# equivalent is CET, which comes from -fcf-protection below.
# -fstack-clash-protection probes each stack page when a large frame is
# allocated, so a file-controlled allocation size cannot jump the guard page.
# -ftrivial-auto-var-init=zero zero-fills uninitialised locals, removing a
# whole class of stack info-leak and uninitialised-read bugs. Both matter here
# because demuxers size buffers from values read out of the media file.
ARCH_FLAGS="-march=$MARCH -fcf-protection=full -fstack-clash-protection -ftrivial-auto-var-init=zero"

# Link-time hardening. meson's default gives partial RELRO only; -z now resolves
# every symbol at load so the GOT can be mapped read-only, which closes off GOT
# overwrite as an exploitation path. -z noexecstack is belt-and-braces.
LINK_FLAGS="-Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack"

# ------------------------------------------------------------------ hardening
# _FORTIFY_SOURCE and stack protector come from ssp; branch_protection enables
# the CPU's indirect-branch hardening where the target supports it.
HARDENING="
-Dssp=enabled
-Drun_as_root=false
-Dtests=disabled
-Dnls=disabled
-Drust=disabled
"

# shellcheck disable=SC2086
exec meson setup "$BUILDDIR" "$SRCDIR" \
    --pkg-config-path="$CONTRIB/lib/pkgconfig" \
    --buildtype=release \
    -Db_lto=true \
    -Db_pie=true \
    -Dc_args="$ARCH_FLAGS" \
    -Dcpp_args="$ARCH_FLAGS" \
    -Dc_link_args="$LINK_FLAGS" \
    -Dcpp_link_args="$LINK_FLAGS" \
    $NET $REMOTE $ENCODERS $CAPTURE $UIS $MISC $ARCH $HARDENING \
    "$@"
