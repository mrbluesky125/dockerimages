TEMPLATE = app

QT += quick qml
SOURCES += main.cpp
RESOURCES += \
    imageparticle.qrc \
    ../images.qrc \
    ../../shared/shared.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/particles/imageparticle
INSTALLS += target
