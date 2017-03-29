TARGET = multitouchwindow

SOURCES += $$PWD/main.cpp \

QT += quick widgets

target.path = $$[QT_INSTALL_EXAMPLES]/window/multitouchwindow
INSTALLS += target

RESOURCES += multitouchwindow.qrc \
