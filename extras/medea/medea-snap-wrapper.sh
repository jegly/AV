#!/bin/bash
# The KDE content snap ships a VLC of its own and sets a plugin path for it.
# Put ours first and drop theirs, or Medea loads the wrong plugins.
export LD_LIBRARY_PATH="$SNAP/usr/lib/x86_64-linux-gnu:$SNAP/usr/lib/x86_64-linux-gnu/medea:$SNAP/usr/lib/x86_64-linux-gnu/pulseaudio:$SNAP/usr/lib/x86_64-linux-gnu/libproxy:$SNAP/usr/lib:$SNAP/usr/lib/medea:$LD_LIBRARY_PATH"
unset VLC_PLUGIN_PATH

export KDE_FORK_SLAVES=1

# libva + wayland is broken against the gpu2404 base snap; force xcb.
export QT_QPA_PLATFORM=xcb

# So the host shell can resolve the tray icon by name over StatusNotifierItem.
export XDG_DATA_DIRS="$SNAP/usr/share:$XDG_DATA_DIRS"

exec "$SNAP/usr/bin/medea" --config="$SNAP_USER_COMMON/medearc" "$@"
