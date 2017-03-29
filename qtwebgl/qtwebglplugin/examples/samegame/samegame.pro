TEMPLATE = app

QT += qml quick sql
SOURCES += main.cpp
RESOURCES += samegame.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/samegame
INSTALLS += target

!qtConfig(sql-sqlite): QTPLUGIN += qsqlite
