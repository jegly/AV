/*****************************************************************************
 * Copyright (C) 2020 VLC authors and VideoLAN
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
import QtQuick
import QtQuick.Window

import VLC.MainInterface
import VLC.Widgets as Widgets
import VLC.Util
import VLC.Playlist
import VLC.Style
import VLC.Dialogs

FocusScope {
    id: root

    property var pagePrefix: [] // behave like a Page

    readonly property ColorContext colorContext: ColorContext {
        id: theme
        colorSet: ColorContext.View
    }

    Accessible.role: Accessible.Client
    Accessible.name: qsTr("Home View")


    component ConeNButtons: FocusScope {
        id: coneNButtons

        property int orientation: Qt.Vertical

        property real spacing: VLCStyle.margin_large

        implicitWidth: orientation === Qt.Vertical ? Math.max(cone.implicitWidth, buttons.implicitWidth)
                                                   : cone.implicitWidth + spacing + buttons.implicitWidth
        implicitHeight: orientation === Qt.Vertical ? cone.implicitHeight + spacing + buttons.implicitHeight
                                                    : cone.implicitHeight

        states: [
            State {
                name: "vertical"
                AnchorChanges {
                    target: cone

                    anchors.left: undefined
                    anchors.horizontalCenter: coneNButtons.horizontalCenter
                }
                AnchorChanges {
                    target: buttons

                    anchors.left: undefined
                    anchors.verticalCenter: undefined
                    anchors.top: cone.bottom
                    anchors.horizontalCenter: coneNButtons.horizontalCenter
                }
                PropertyChanges {
                    target: buttons

                    anchors.topMargin: coneNButtons.spacing
                }
            },
            State {
                name: "horizontal"
                AnchorChanges {
                    target: cone

                    anchors.horizontalCenter: undefined
                    anchors.left: coneNButtons.left
                }
                AnchorChanges {
                    target: buttons

                    anchors.top: undefined
                    anchors.horizontalCenter: undefined
                    anchors.left: cone.right
                    anchors.verticalCenter: coneNButtons.verticalCenter
                }
                PropertyChanges {
                    target: buttons

                    anchors.leftMargin: coneNButtons.spacing
                }
            }
        ]

        state: orientation === Qt.Vertical ? "vertical" : "horizontal"


        Image {
            id: cone

            // Hidden on the populated Home page (corner badge); still shown
            // on the empty-library placeholder (large centered logo).
            visible: orientation === Qt.Vertical

            property real _eDPR: MainCtx.effectiveDevicePixelRatio(Window.window)

            sourceSize: Qt.size(0, orientation === Qt.Vertical ? VLCStyle.colWidth(1)
                                                               : VLCStyle.icon_large * _eDPR)

            // Medea's mark, not VideoLAN's cone - see BannerCone.qml.
            source: "qrc:///logo/medea.svg"

            Connections {
                target: MainCtx

                function onIntfDevicePixelRatioChanged() {
                    // Update the DPR:
                    // Normally, this is not done, as we display the images at the size we
                    // want, and we don't want to re-load all images on DPR change. But
                    // in this case we depend on the implicit size, so we should re-load
                    // the image with the updated DPR:
                    cone._eDPR = MainCtx.effectiveDevicePixelRatio(cone.Window.window)
                }
            }
        }

        // The "Open File" button used to live here. It never worked (the
        // dialog silently failed to appear regardless of which backend was
        // tried - portal, non-native Qt dialog, transient-parented Qt
        // dialog), so it was removed rather than ship a dead button. Use the
        // Media menu or Ctrl+O instead.
        //
        // `buttons` is kept as an empty anchor target: the states above
        // reference it by id for layout, and coneNButtons.implicitWidth/Height
        // read its (now zero) size.
        Row {
            id: buttons

            spacing: coneNButtons.spacing
        }
    }


    ConeNButtons {
        focus: true

        anchors.centerIn: root

        Navigation.parentItem: root
    }
}
