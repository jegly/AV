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

exec "$SNAP/usr/bin/av" --config="$SNAP_USER_COMMON/avrc" "$@"
