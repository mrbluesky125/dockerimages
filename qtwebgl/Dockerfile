FROM qtbase_image
  
ENV APP_PORT 33334
  
RUN cd /qt5 && git clone http://code.qt.io/qt/qtwebglplugin.git && cd /qt5/qtwebglplugin && \
    sed -ie 's/bool ok = m_server->listen(QHostAddress::Any);/bool ok = m_server->listen(QHostAddress::Any, 33334);/g;' /qt5/qtwebglplugin/src/plugins/platforms/webgl/qwebglwebsocketserver.cpp && \
    sed -ie 's/requires(!winrt:qtHaveModule(websockets):qtHaveModule(gui):qtConfig(opengles2));requires(!winrt:qtHaveModule(websockets):qtHaveModule(gui))/g' /qt5/qtwebglplugin/qtwebglplugin.pro && \
    syncqt.pl -version $QT_VERSION && qmake && make -j32 && make install && \
#    cp plugins/platforms/libqwebgl.so /usr/local/Qt-$QT_VERSION/plugins/platforms/libqwebgl.so && \
    updatedb

# For Qt WebGL
EXPOSE 8080
# QWebSocketServer port
EXPOSE 33334
