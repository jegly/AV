/*****************************************************************************
 * Copyright (C) 2024 VLC authors and VideoLAN
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * ( at your option ) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#include "systray.hpp"

#include <QImageReader>
#include <QPixmap>
#include <QIcon>

#include "maininterface/mainctx.hpp"
#include "menus/menus.hpp"
#include "playlist/playlist_controller.hpp"
#include "player/player_controller.hpp"
#include "dialogs/dialogs_provider.hpp"
#include "widgets/native/qvlcframe.hpp"

using namespace vlc::playlist;

VLCSystray::VLCSystray(MainCtx* ctx, QObject* parent)
    : QSystemTrayIcon(parent)
    , m_ctx(ctx)
    , m_intf(ctx->getIntf())
{
    assert(ctx);
    assert(m_intf);

    m_notificationSetting = var_InheritInteger(m_intf, "qt-notification");

    /* StatusNotifierItem hands the host shell either an icon *name* to resolve
     * against its own theme, or serialised pixmap data. A name only works if
     * av.svg is installed in a system icon theme, which is not the case for
     * an uninstalled/staged build or inside a snap - the host then shows a
     * placeholder ("..." on GNOME).
     *
     * Building the icon from explicit pixmaps forces Qt to send the image data
     * itself, so the tray renders correctly regardless of the host's theme. */
    QIcon iconMedea = QIcon::fromTheme( QStringLiteral( "av" ) );

    if( iconMedea.isNull() || iconMedea.availableSizes().isEmpty() )
    {
        iconMedea = QIcon();
        const QIcon source( QStringLiteral( ":/logo/av.svg" ) );
        for( int size : { 16, 22, 24, 32, 48, 64, 128, 256 } )
        {
            const QPixmap pm = source.pixmap( QSize( size, size ) );
            if( !pm.isNull() )
                iconMedea.addPixmap( pm );
        }

        if( iconMedea.availableSizes().isEmpty() )
            msg_Warn( m_intf, "could not build the AV tray icon from "
                              ":/logo/av.svg" );
    }

    setIcon( iconMedea );
    setToolTip( qtr( "AV" ));

    m_menu = std::make_unique<VLCMenu>( qtr( "AV"), m_intf );
    m_menu->setIcon( iconMedea );
    setContextMenu(m_menu.get());
    update();
    show();

    connect( this, &QSystemTrayIcon::activated,
            this, &VLCSystray::handleClick );

    /* Connects on nameChanged() */
    connect( m_intf->p_mainPlayerController, &PlayerController::nameChanged,
            this, &VLCSystray::updateTooltipName );
    /* Connect PLAY_STATUS on the systray */
    connect( m_intf->p_mainPlayerController, &PlayerController::playingStateChanged,
            this, &VLCSystray::update );
}

VLCSystray::~VLCSystray()
{}

/**
 * Updates the VLCSystray Icon's menu and toggle the main interface
 */
void VLCSystray::toggleUpdateMenu()
{
    m_ctx->toggleWindowVisibility();
    update();
}

/* First Item of the systray menu */
void VLCSystray::showUpdateMenu()
{
    m_ctx->setInterfaceVisibible(true);
    update();
}

/* First Item of the systray menu */
void VLCSystray::hideUpdateMenu()
{
    m_ctx->setInterfaceVisibible(false);
    update();
}

/* Click on systray Icon */
void VLCSystray::handleClick(
    QSystemTrayIcon::ActivationReason reason )
{
    switch( reason )
    {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        toggleUpdateMenu();
        break;
    case QSystemTrayIcon::MiddleClick:
        if (PlaylistController* const playlistController = m_intf->p_mainPlaylistController)
            playlistController->togglePlayPause();
        break;
    default:
        break;
    }
}

/**
 * Updates the name of the systray Icon tooltip.
 * Doesn't check if the systray exists, check before you call it.
 **/
void VLCSystray::updateTooltipName( const QString& name )
{
    if( name.isEmpty() )
    {
        setToolTip( qtr( "AV" ) );
    }
    else
    {
        setToolTip( name );
        const auto windowVisiblity = m_ctx->interfaceVisibility();
        if( ( m_notificationSetting == NOTIFICATION_ALWAYS ) ||
            ( m_notificationSetting == NOTIFICATION_MINIMIZED && (windowVisiblity == QWindow::Hidden || windowVisiblity == QWindow::Minimized)))
        {
            const auto showMessageTemplate = [this, &name](const auto &icon) {
                showMessage( qtr( "AV" ), name, icon, 3000 );
            };

            assert(m_intf);
            assert(m_intf->p_mainPlayerController);
            const QUrl& art = m_intf->p_mainPlayerController->getArtwork();
            if (art.isValid())
            {
                // If there is artwork, use it but only when it is (almost) square:
                const QString& fileName = art.toLocalFile();
                const QImageReader imageReader(fileName);
                if (Q_LIKELY(imageReader.canRead()))
                {
                    const QSize& size = imageReader.size(); // this does not read the whole image
                    if (Q_LIKELY(!size.isEmpty()))
                    {
                        const double ratio = static_cast<double>(size.width()) / size.height();
                        if (Q_LIKELY(std::abs(ratio - 1.0) < 0.2))
                        {
                            showMessageTemplate(QIcon(fileName));
                            return;
                        }
                    }
                }
            }

            showMessageTemplate( QSystemTrayIcon::NoIcon );
        }
    }
    update();
}

void VLCSystray::update()
{
    // explictly delete submenus, see QTBUG-11070
    for (QAction *action : m_menu->actions()) {
        if (action->menu()) {
            delete action->menu();
        }
    }
    m_menu->clear();

#ifndef Q_OS_MAC
    /* Hide / Show VLC and cone */
    if( m_ctx->interfaceVisibility() != QWindow::Hidden )
    {
        m_menu->addAction(
            QIcon( ":/logo/av.svg" ), qtr( "&Hide AV in taskbar" ),
            this, &VLCSystray::hideUpdateMenu);
    }
    else
    {
        m_menu->addAction(
            QIcon( ":/logo/av.svg" ), qtr( "Sho&w AV" ),
            this, &VLCSystray::showUpdateMenu);
    }
    m_menu->addSeparator();
#endif

    VLCMenuBar::PopupMenuPlaylistEntries( m_menu.get(), m_intf );
    VLCMenuBar::PopupMenuControlEntries( m_menu.get(), m_intf, false );

    VLCMenuBar::VolumeEntries( m_intf, m_menu.get() );
    m_menu->addSeparator();
    m_menu->addAction(
        QIcon(":/menu/file.svg"), qtr( "&Open Media" ),
        THEDP, &DialogsProvider::openFileDialog);

    m_menu->addAction(
        QIcon(":/menu/exit.svg"), qtr( "&Quit" ),
        THEDP, &DialogsProvider::quit);

    /* Set the menu */
    setContextMenu( m_menu.get() );
}
