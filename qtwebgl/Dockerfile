FROM qtbase_image

ENV DEBIAN_FRONTEND noninteractive
  
ADD qtwebglplugin /qt5/qtwebglplugin

ADD supervisord.conf /etc/supervisor/conf.d/supervisord.conf
    
RUN cd /qt5/qtwebglplugin && \
    sed -ie 's/bool ok = m_server->listen(QHostAddress::Any);/bool ok = m_server->listen(QHostAddress::Any, 33334);/g;' /qt5/qtwebglplugin/src/plugins/platforms/webgl/qwebglwebsocketserver.cpp && \
    /usr/local/Qt-5.9.0/bin/syncqt.pl -version 5.9 && \
    /usr/local/Qt-5.9.0/bin/qmake && \
    make -j32 && \
    cp plugins/platforms/libqwebgl.so /usr/local/Qt-5.9.0/plugins/platforms/libqwebgl.so && \
    updatedb
    
#SSH
EXPOSE 22

# For Qt WebGL
EXPOSE 8080
# QWebSocketServer port
EXPOSE 33334

WORKDIR /root

ADD start.sh /usr/local/start.sh
RUN chmod 777 /usr/local/start.sh

USER root

CMD ["/usr/local/start.sh"]
