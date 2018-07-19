FROM mrbluesky125/ubuntubase

ENV DEBIAN_FRONTEND noninteractive
ENV QT_VERSION_TAG 5.10.0

RUN cd /opt && git clone https://code.qt.io/qt/qt5.git && \
    cd qt5 && git checkout tags/v$QT_VERSION_TAG && \ 
    ./init-repository --module-subset=default,-qtwebkit,-qtwebkit-examples,-qtwebengine
    
WORKDIR /root
