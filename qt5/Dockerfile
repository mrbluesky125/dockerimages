FROM mrbluesky125/qtbase

ENV DEBIAN_FRONTEND noninteractive
ENV QT_VERSION_TAG 5.10.0

RUN cd /opt/qt5 && ./configure -opensource -confirm-license -nomake examples -nomake tests --silent && \
    sed -ie 's/bool ok = d->server->listen(QHostAddress::Any);/bool ok = d->server->listen(QHostAddress::Any, qgetenv("WEBGL_PORT").toInt());/g' /opt/qt5/qtwebglplugin/src/plugins/platforms/webgl/qwebglwebsocketserver.cpp && \
    sed -ie 's/if (d->server->listen(QHostAddress::Any)) {/if (d->server->listen(QHostAddress::Any, qgetenv("WEBGL_PORT").toInt())) {/g' /opt/qt5/qtwebglplugin/src/plugins/platforms/webgl/qwebglwebsocketserver.cpp && \
    make -j$(grep -c ^processor /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu) && make install && \
    git submodule foreach --recursive "git clean -dfx" && git clean -dfx

EXPOSE 8080
ENV WEBGL_PORT 8081
ENV PATH /usr/local/Qt-$QT_VERSION_TAG/bin:$PATH
ENV LD_LIBRARY_PATH /usr/local/Qt-$QT_VERSION_TAG/lib:$LD_LIBRARY_PATH

RUN echo 'export QT_VERSION='QT_VERSION_TAG >> /etc/bash.bashrc
RUN echo 'export PATH='$PATH >> /etc/bash.bashrc
RUN echo 'export LD_LIBRARY_PATH='$LD_LIBRARY_PATH >> /etc/bash.bashrc

WORKDIR /root
