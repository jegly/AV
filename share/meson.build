# Desktop integration files.
#
# Upstream VLC only installs these from the autotools build; the meson port
# never did. Medea needs them installed for two reasons: the .desktop entry is
# what puts the app in the launcher, and the icon has to land in the hicolor
# theme so the *host* shell can resolve "medea" by name over StatusNotifierItem
# (see dialogs/systray/systray.cpp - without this the tray renders a
# placeholder box).

install_data(
    'medea.desktop',
    install_dir: get_option('datadir') / 'applications',
)

install_data(
    'icons/medea.svg',
    install_dir: get_option('datadir') / 'icons' / 'hicolor' / 'scalable' / 'apps',
)

install_data(
    'org.medea.player.metainfo.xml',
    install_dir: get_option('datadir') / 'metainfo',
)

install_data(
    'fonts/DotGothic16-Regular.ttf',
    install_dir: get_option('datadir') / 'fonts' / 'medea',
)
