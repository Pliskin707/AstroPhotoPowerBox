import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12

import pliskin.astroboxgui.backend 1.0

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("AstroBox GUI")

    ScrollView {
        anchors.fill: parent

        Text {
            id: debugText
            text: backend.debugText
        }
    }
}
