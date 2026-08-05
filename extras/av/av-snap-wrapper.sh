#!/bin/bash
# The KDE content snap ships a VLC of its own and sets a plugin path for it.
# Put ours first and drop theirs, or AV loads the wrong plugins.
export LD_LIBRARY_PATH="$SNAP/usr/lib/x86_64-linux-gnu:$SNAP/usr/lib/x86_64-linux-gnu/av:$SNAP/usr/lib/x86_64-linux-gnu/pulseaudio:$SNAP/usr/lib/x86_64-linux-gnu/libproxy:$SNAP/usr/lib:$SNAP/usr/lib/av:$LD_LIBRARY_PATH"
unset VLC_PLUGIN_PATH

export KDE_FORK_SLAVES=1

# So the host shell can resolve the tray icon by name over StatusNotifierItem.
export XDG_DATA_DIRS="$SNAP/usr/share:$XDG_DATA_DIRS"

# alsa-lib looks for its config at the literal absolute path
# /usr/share/alsa/alsa.conf, which under strict confinement is $SNAP's copy,
# not the host's - point it there directly instead.
export ALSA_CONFIG_PATH="$SNAP/usr/share/alsa/alsa.conf"

# Under strict confinement, $XDG_RUNTIME_DIR is remapped to a private
# per-snap directory (e.g. /run/user/1000/snap.av) that does not contain
# the real Wayland socket - the `wayland` interface grants access to it,
# but nothing creates this compatibility symlink automatically. Without
# it, Qt's wayland platform plugin fails with "Failed to create wl_display
# (No such file or directory)" and silently falls back to xcb. This is
# the same logic snapcraft-desktop-helpers' common/desktop-exports uses,
# which is what upstream VLC's own snap wrapper relies on.
if [ -n "${XDG_RUNTIME_DIR:-}" ] && [ -z "${DISABLE_WAYLAND:-}" ]; then
    wdisplay="${WAYLAND_DISPLAY:-wayland-0}"
    wayland_sockpath="$XDG_RUNTIME_DIR/../$wdisplay"
    wayland_snappath="$XDG_RUNTIME_DIR/$wdisplay"
    if [ -S "$wayland_sockpath" ] && [ ! -e "$wayland_snappath" ]; then
        ln -s "$wayland_sockpath" "$wayland_snappath"
    fi
fi

# The PulseAudio socket needs the same treatment as the Wayland one above, and
# for the same reason: libpulse looks for $XDG_RUNTIME_DIR/pulse/native, but
# under strict confinement that resolves to the private per-snap directory,
# which contains an empty `pulse/` dir and no socket - hence "PulseAudio server
# connection failure: Connection refused". The `pulseaudio`/`audio-playback`
# interfaces do grant access to the real socket one level up; nothing points
# libpulse at it. Unlike Wayland this cannot be a symlink into the snap dir -
# libpulse takes a server address - so export PULSE_SERVER instead.
#
# This is byte-for-byte what upstream VLC's own snap ends up with: it gets
# PULSE_SERVER=unix:/run/user/1000/snap.vlc/../pulse/native from the kde-neon-6
# extension's command-chain (kf6-core24's snap/command-chain/desktop-launch6),
# which AV cannot use on core26 and so has to do itself.
#
# Note there is deliberately no ALSA equivalent here. ALSA cannot work under
# this confinement at all - /proc/asound is denied, so alsa-lib cannot even
# enumerate card 0 - and forcing `--aout alsa` fails identically on upstream
# VLC's snap. VLC only falls back to ALSA when pulse fails to open, so with
# this set it is never reached.
if [ -n "${XDG_RUNTIME_DIR:-}" ]; then
    pulseaudio_sockpath="$XDG_RUNTIME_DIR/../pulse/native"
    if [ -S "$pulseaudio_sockpath" ]; then
        export PULSE_SERVER="unix:${pulseaudio_sockpath}"
    fi
fi

exec "$SNAP/usr/bin/av" --config="$SNAP_USER_COMMON/avrc" "$@"
