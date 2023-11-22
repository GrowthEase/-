﻿import QtQuick
import Qt5Compat.GraphicalEffects

Rectangle {
    property string img_src

    Image {
        id: _image
        smooth: true
        mipmap: true
        visible: false
        anchors.fill: parent
        source: img_src
        sourceSize: Qt.size(parent.size, parent.size)
        antialiasing: true
    }
    Rectangle {
        id: _mask
        color: "white"
        anchors.fill: parent
        radius: parent.radius
        visible: false
        antialiasing: true
        smooth: true
    }
    OpacityMask {
        id: mask_image
        anchors.fill: _image
        source: _image
        maskSource: _mask
        visible: true
        antialiasing: true
    }
}
