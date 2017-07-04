FROM qtbase_image

#Default port  
ENV WEBGL_PORT 8081
  
RUN cd /qt5 && git clone http://code.qt.io/qt/qtwebglplugin.git && cd /qt5/qtwebglplugin && \
    sed -ie 's/bool ok = d->server->listen(QHostAddress::Any);/bool ok = d->server->listen(QHostAddress::Any, qgetenv("WEBGL_PORT").toInt());/g' /qt5/qtwebglplugin/src/plugins/platforms/webgl/qwebglwebsocketserver.cpp && \
    sed -ie 's/requires(!winrt:qtHaveModule(websockets):qtHaveModule(gui):qtConfig(opengles2))/requires(!winrt:qtHaveModule(websockets):qtHaveModule(gui))/g' /qt5/qtwebglplugin/qtwebglplugin.pro && \
    syncqt.pl -version $QT_VERSION && qmake && make -j32 && make install && \
    updatedb

#Connect port
EXPOSE 8080

