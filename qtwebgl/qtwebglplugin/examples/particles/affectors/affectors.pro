TEMPLATE = app

QT += quick qml
SOURCES += main.cpp
RESOURCES += \
    affectors.qrc \
    ../images.qrc \
    ../../shared/shared.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/particles/affectors
INSTALLS += target
