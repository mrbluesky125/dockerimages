import QtQuick 2.6
import QtGraphicalEffects 1.0
import QtQuick.Controls 1.4
import QtCharts 2.0
import QtQuick.Dialogs 1.2

Rectangle {
    id: root
    property bool running: true
    property int loops: -1
    color: "transparent"

    Image {
        id: qtLogo
        width: sourceSize.width / 4
        height: sourceSize.height / 4
        x: parent.width / 2 - width / 2
        y: parent.height / 2 - height / 2
        source: "qt.png"

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            drag.target: qtLogo
            drag.minimumX: 0
            drag.maximumX: root.width - qtLogo.width
            drag.onActiveChanged: {
                console.log("active changed: " + drag.active)
            }

            onClicked: {
                console.log("Clicked!!");
            }
        }
    }

    Column {
        id: buttonColumn
        anchors.bottom: qtLogo.top
        anchors.horizontalCenter: qtLogo.horizontalCenter

        Button {
            FileDialog {
                id: fileDialog
                title: "Please choose a file"
                folder: shortcuts.home
                onAccepted: close()
                onRejected: close()
            }

            text: fileDialog.title
            onClicked: fileDialog.open()
        }

        Button {
            ColorDialog {
                id: colorDialog
                title: "Please choose a color"
                onAccepted: close()
                onRejected: close()
            }

            text: colorDialog.title
            onClicked: colorDialog.open()
        }

        Button {
            FontDialog {
                id: fontDialog
                title: "Please choose a font"
                onAccepted: close()
                onRejected: close()
            }

            text: fontDialog.title
            onClicked: fontDialog.open()
        }

        Button {
            MessageDialog {
                id: messageDialog
                title: "Hello world!!"
                onAccepted: close()
                onRejected: close()
            }

            text: "MessageDialog"
            onClicked: messageDialog.open()
        }
    }

    Button {
        anchors.top: qtLogo.bottom
        anchors.horizontalCenter: qtLogo.horizontalCenter
        text: "OpenUrl"
        onClicked: Qt.openUrlExternally("https://www.qt.io/");
    }

    ListModel {
        id: libraryModel
        ListElement {
            title: "A Masterpiece"
            author: "Gabriel"
        }
        ListElement {
            title: "Brilliance"
            author: "Jens"
        }
        ListElement {
            title: "Outstanding"
            author: "Frederik"
        }
    }

    TableView {
        id: tableView
        TableViewColumn {
            role: "title"
            title: "Title"
            width: 100
        }
        TableViewColumn {
            role: "author"
            title: "Author"
            width: 200
        }
        model: libraryModel
    }

    TextArea {
        anchors.left: tableView.right
        anchors.leftMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.bottom: buttonColumn.top
        anchors.bottomMargin: 10
        text: "Try writing"
    }

    Text {
        z: 5
        text: parent.width + " x " + parent.height
    }

//    ChartView {
//        id: chart
//        title: "Top-5 car brand shares in Finland"
//        anchors.right: parent.right
//        anchors.bottom: parent.bottom
//        legend.alignment: Qt.AlignBottom
//        antialiasing: true
//        width: 640
//        height: 480

//        PieSeries {
//            id: pieSeries
//            PieSlice { label: "Volkswagen"; value: 13.5 }
//            PieSlice { label: "Toyota"; value: 10.9 }
//            PieSlice { label: "Ford"; value: 8.6 }
//            PieSlice { label: "Skoda"; value: 8.2 }
//            PieSlice { label: "Volvo"; value: 6.8 }
//        }
//    }

//    Component.onCompleted: {
//        pieSeries.find("Volkswagen").exploded = true;
//    }

//    anchors.fill: parent

//    Image {
//        source: "qt.png"
//        anchors.fill: parent
//    }

//    Image {
//        id: qtLogo
//        width: sourceSize.width / 4
//        height: sourceSize.height / 4
//        source: "qt.png"
//        anchors.centerIn: parent
//    }

//    ZoomBlur {
//        anchors.fill: qtLogo
//        source: qtLogo
//        samples: 24
//        length: 20
//    }

//    Rectangle {
//        id: rectangle
//        color: "red"
//        width: 320;
//        height: 240

//        NumberAnimation on rotation {
//            running: root.running
//            from: 0
//            to: 360
//            duration: 5000
//            loops: root.loops
//        }
//    }

//    Column {
//        spacing: 40
//        width: parent.width
//        y: 300

//        Label {
//            width: parent.width
//            wrapMode: Label.Wrap
//            horizontalAlignment: Qt.AlignHCenter
//            text: "Tumbler is used to select a value by spinning a wheel."
//        }

//        Tumbler {
//            model: 10
//            visibleItemCount: 5
//            anchors.horizontalCenter: parent.horizontalCenter
//        }
//    }

//    Dial {
//        x: 200
//        height: 200
//        value: 0.5
//        anchors.horizontalCenter: parent.horizontalCenter
//    }

//    Rectangle {
//        color: "yellow"
//        width: 100
//        height: 100

//        NumberAnimation on x {
//            running: root.running
//            from: 0
//            to: 640
//            duration: 5000
//            loops: root.loops
//        }

//        NumberAnimation on y {
//            running: root.running
//            from: 0
//            to: 480
//            duration: 5000
//            loops: root.loops
//        }
//    }

//    Text {
//        id: helloText
//        text: "Hello Qt!!"
//        anchors.centerIn: parent
//        color: "blue"
//        font.pixelSize: 32
//        renderType: Text.NativeRendering
//    }

//    Rectangle {
//        color: "white"
//        width: resolutionText.paintedWidth * 2
//        height: resolutionText.paintedHeight * 2

//        Text {
//            id: resolutionText
//            anchors.centerIn: parent
//            text: root.width + "x" + root.height
//        }
//    }
}
