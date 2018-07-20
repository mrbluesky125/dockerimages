FROM mrbluesky125/ubuntubase

ENV DEBIAN_FRONTEND noninteractive
ENV QT_VERSION_TAG 5.10.0

RUN cd /opt && git clone https://code.qt.io/qt/qt5.git && \
    cd qt5 && git checkout tags/v$QT_VERSION_TAG && \ 
    ./init-repository --module-subset=default,-qtwebkit,-qtwebkit-examples,-qtwebengine && \
	./configure -opensource -confirm-license -nomake examples -nomake tests --silent && \
    sed -ie 's/bool ok = d->server->listen(QHostAddress::Any);/bool ok = d->server->listen(QHostAddress::Any, qgetenv("WEBGL_PORT").toInt());/g' /opt/qt5/qtwebglplugin/src/plugins/platforms/webgl/qwebglwebsocketserver.cpp && \
    sed -ie 's/if (d->server->listen(QHostAddress::Any)) {/if (d->server->listen(QHostAddress::Any, qgetenv("WEBGL_PORT").toInt())) {/g' /opt/qt5/qtwebglplugin/src/plugins/platforms/webgl/qwebglwebsocketserver.cpp
    
WORKDIR /root
