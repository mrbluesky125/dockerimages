/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.0
import QtQuick.Particles 2.0

Item {
    z: 10
    property alias source: img.source
    property alias system: emitter.system
    property int playerNum: 1
    function play() {
        anim.running = true;
    }
    anchors.centerIn: parent
    Image {
        opacity: 0
        id: img
        anchors.centerIn: parent
        rotation: playerNum == 1 ? -8 : -5
        Emitter {
            id: emitter
            group: "smoke"
            anchors.fill: parent
            shape: MaskShape { source: img.source }
            enabled: false
            emitRate: 1000
            lifeSpan: 600
            size: 64
            endSize: 32
            velocity: AngleDirection { angleVariation: 360; magnitudeVariation: 160 }
        }
    }
    SequentialAnimation {
        id: anim
        running: false
        PauseAnimation { duration: 500}
        ParallelAnimation {
            NumberAnimation { target: img; property: "opacity"; from: 0.1; to: 1.0 }
            NumberAnimation { target: img; property: "scale"; from: 0.1; to: 1.0 }
        }
        PauseAnimation { duration: 250}
        ScriptAction { script: emitter.pulse(100); }
        NumberAnimation { target: img; property: "opacity"; from: 1.0; to: 0.0 }
    }
}
