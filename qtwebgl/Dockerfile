FROM qtbase_image

ENV DEBIAN_FRONTEND noninteractive
  
RUN cd /qt5 && git clone http://code.qt.io/qt/qtwebglplugin.git && cd /qt5/qtwebglplugin && \
    sed -ie 's/bool ok = m_server->listen(QHostAddress::Any);/bool ok = m_server->listen(QHostAddress::Any, 33334);/g;' /qt5/qtwebglplugin/src/plugins/platforms/webgl/qwebglwebsocketserver.cpp && \
    syncqt.pl -version $QT_VERSION && qmake && \
    make -j32 && \
    cp plugins/platforms/libqwebgl.so /usr/local/Qt-$QT_VERSION/plugins/platforms/libqwebgl.so && \
    updatedb

ADD supervisord.conf /etc/supervisor/conf.d/supervisord.conf
    
#SSH
EXPOSE 22

# For Qt WebGL
EXPOSE 8080
# QWebSocketServer port
EXPOSE 33334

ADD start.sh /usr/local/start.sh
RUN chmod 777 /usr/local/start.sh

CMD ["/usr/local/start.sh"]

