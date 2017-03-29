TARGET = qwebgl
QT += platformcompositor_support-private \
    theme_support-private \
    #service_support-private \
    fontdatabase_support-private \
    eventdispatcher_support-private \
    gui-private \
    gui \
    websockets

HEADERS += \
    qwebglcontext.h \
    qwebglfunctioncall.h \
    qwebglintegration.h \
    qwebgloffscreenwindow.h \
    qwebglscreen.h \
    qwebglwindow.h \
#    qwebglbackingstore.h
    qwebglhttpserver.h \
    qwebglwebsocketserver.h \
    qwebglplatformservices.h \
    qwebglplatformclipboard.h

SOURCES += \
    qwebglcontext.cpp \
    qwebglfunctioncall.cpp \
    qwebglintegration.cpp \
    qwebglmain.cpp \
    qwebgloffscreenwindow.cpp \
    qwebglscreen.cpp \
    qwebglwindow.cpp \
#    qwebglbackingstore.cpp
    qwebglhttpserver.cpp \
    qwebglwebsocketserver.cpp \
    qwebglplatformservices.cpp \
    qwebglplatformclipboard.cpp

OTHER_FILES += $$files(*.json)
PLUGIN_TYPE = platforms
PLUGIN_CLASS_NAME = QWebGLIntegrationPlugin
!equals(TARGET, $$QT_DEFAULT_QPA_PLUGIN): PLUGIN_EXTENDS = -
load(qt_plugin)

RESOURCES += \
    webgl.qrc
