/*****************************************************************************
 * Copyright (C) 2020 VLC authors and VideoLAN
 * Copyright (C) 2026 Medea authors
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
import QtQuick.Templates as T

import VLC.Widgets as Widgets
import VLC.MainInterface
import VLC.Style


// Traffic-light window button: a filled circle that reveals its glyph only
// while the button group is hovered, the way macOS does it. Upstream drew a
// full-height rectangle with a permanently visible icon, which is the most
// dated element of the window chrome.
T.Button {
    id: control

    // Fill colour of the dot; set per button type by CSDWindowButtonSet.
    property color dotColor: "#8A8A8E"

    property color color
    property color hoverColor
    property string iconTxt: ""
    property bool showHovered: false
    property bool isThemeDark: false
    property bool externalPressed: false

    readonly property bool _paintHovered: control.hovered || showHovered

    readonly property int dotSize: VLCStyle.dp(13, VLCStyle.scale)

    padding: 0

    width: dotSize + VLCStyle.dp(9, VLCStyle.scale)
    implicitWidth: width
    implicitHeight: dotSize

    focusPolicy: Qt.NoFocus

    background: Item {}

    contentItem: Item {
        Rectangle {
            id: dot

            anchors.centerIn: parent
            width: control.dotSize
            height: control.dotSize
            radius: width / 2

            color: (control.pressed || control.externalPressed)
                   ? Qt.darker(control.dotColor, 1.25)
                   : control.dotColor

            // A hairline keeps the dots legible on backgrounds close to their
            // own colour (Mono Red, Grass, Borland...).
            border.width: 1
            border.color: control.isThemeDark ? Qt.rgba(0, 0, 0, 0.22)
                                              : Qt.rgba(0, 0, 0, 0.14)

            Behavior on color {
                ColorAnimation { duration: 75 }
            }

            Widgets.IconLabel {
                anchors.centerIn: parent
                text: control.iconTxt

                font.family: VLCIcons.fontFamily
                font.pixelSize: Math.round(control.dotSize * 0.62)

                // Glyphs are dark so they read against the bright dot fills.
                color: Qt.rgba(0, 0, 0, 0.62)

                opacity: control._paintHovered ? 1.0 : 0.0
                visible: opacity > 0

                Behavior on opacity {
                    NumberAnimation { duration: 75 }
                }
            }
        }
    }
}
