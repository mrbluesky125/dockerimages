TARGET = quickwindow

QT += quick widgets

SOURCES += main.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/window/quickwindow
INSTALLS += target

RESOURCES += \
    quickwindow.qrc
