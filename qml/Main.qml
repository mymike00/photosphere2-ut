/*
 * Copyright (C) 2020  Michele Castellazzi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * photosphere2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Suru 2.2
import QtQuick.Layouts 1.3
import Qt.labs.settings 1.0

import PhotoSphere 1.0

ApplicationWindow {
    id: root
    objectName: 'mainView'
    visible: true

    width: Suru.units.gu(100)
    height: Suru.units.gu(75)

    Page {
        anchors.fill: parent
        background: null
        // header: ToolBar {
        //     id: header
        //     Label {
        //         text: "PhotoSphere 2"
        //         font.pixelSize: Suru.units.gu(3)
        //         elide: Label.ElideRight
        //         anchors {
        //             verticalCenter: parent.verticalCenter
        //             left: parent.left
        //             leftMargin: Suru.units.gu(1)
        //         }
        //     }
        // }

        PhotoSphere {
            id: photoSphere
            image: ":/assets/grid2.png"
            scale: 1
            onScaleChanged: {
                scale = Math.min(scale, 10)
                scale = Math.max(scale, 0.1)
            }
        }
        PinchArea {
            id: pa
            anchors.fill: parent
            property var clickedScale
            property var rotation
            onPinchStarted: {
                clickedScale = photoSphere.scale
            }
            onPinchUpdated: {
                photoSphere.scale = pinch.scale * clickedScale
            }
            onPinchFinished: {
                photoSphere.rotateView(rotation)
            }
            MouseArea {
                id: ma
                anchors.fill: parent
                onWheel: {
                    photoSphere.scale -= photoSphere.scale * wheel.angleDelta.y / 200 / Suru.units.dp(1)
                }
            }
        }
    }
    function round(num) {
        return Math.round(num * 10) / 10
    }
}
