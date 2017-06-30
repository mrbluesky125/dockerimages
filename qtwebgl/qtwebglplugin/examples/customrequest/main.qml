import QtQuick 2.7
import QtQuick.Controls 2.0

Item {
    width: 1024
    height: 768

    Button {
        width: 192
        height: 32
        anchors.centerIn: parent
        text: "Download file"
        onClicked: {
            Qt.openUrlExternally("http://127.0.0.1:8080/test.csv");
        }
    }
}
