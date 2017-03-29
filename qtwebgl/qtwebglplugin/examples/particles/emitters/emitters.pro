TEMPLATE = app

QT += quick qml
SOURCES += main.cpp
RESOURCES += \
    emitters.qrc \
    ../images.qrc \
    ../../shared/shared.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/particles/emitters
INSTALLS += target
