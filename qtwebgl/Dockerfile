FROM qtbase_image

ENV DEBIAN_FRONTEND noninteractive
  
ADD qtwebglplugin /qt5/qtwebglplugin

RUN cd /qt5/qtwebglplugin && \
    sed -ie 's/bool ok = m_server->listen(QHostAddress::Any);/bool ok = m_server->listen(QHostAddress::Any, 33334);/g;' /qt5/qtwebglplugin/src/plugins/platforms/webgl/qwebglwebsocketserver.cpp && \
    syncqt.pl -version 5.9 && qmake && \
    make -j32 && \
    cp plugins/platforms/libqwebgl.so /usr/local/Qt-5.9.1/plugins/platforms/libqwebgl.so && \
    updatedb

ADD supervisord.conf /etc/supervisor/conf.d/supervisord.conf
    
#SSH
EXPOSE 22

# For Qt WebGL
EXPOSE 8080
# QWebSocketServer port
EXPOSE 33334

WORKDIR /root
USER root

CMD ["/bin/bash", "/usr/bin/supervisord", "-n"]
