import QtQuick 2.7

Rectangle {
    id: root
    color: "transparent"

    MultiPointTouchArea {
              anchors.fill: parent
              touchPoints: [
                  TouchPoint { id: point1 },
                  TouchPoint { id: point2 },
                  TouchPoint { id: point3 },
                  TouchPoint { id: point4 },
                  TouchPoint { id: point5 },
                  TouchPoint { id: point6 }
              ]
          }

          Rectangle {
              width: 30; height: 30
              color: "green"
              x: point1.x
              y: point1.y
          }

          Rectangle {
              width: 30; height: 30
              color: "yellow"
              x: point2.x
              y: point2.y
          }

          Rectangle {
              width: 30; height: 30
              color: "red"
              x: point3.x
              y: point3.y
          }

          Rectangle {
              width: 30; height: 30
              color: "blue"
              x: point4.x
              y: point4.y
          }
          Rectangle {
              width: 30; height: 30
              color: "black"
              x: point5.x
              y: point5.y
          }

          Rectangle {
              width: 30; height: 30
              color: "white"
              x: point6.x
              y: point6.y
          }
}
