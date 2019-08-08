FROM mrbluesky125/ubuntubase

ENV DEBIAN_FRONTEND noninteractive
ENV QT_VERSION_TAG 5.13.0

RUN cd /opt && git clone https://code.qt.io/qt/qt5.git && \
    cd qt5 && git checkout tags/v$QT_VERSION_TAG && \ 
    ./init-repository --module-subset=default,-qtwebview,-qtwebengine,-qtwebchannel,-qt3d,-qtcanvas3d,-qtscript,-qtwayland,-qtspeech,-qtdatavis3d,-qtpurchasing,-qtlocation,-qtgamepad,-qtdoc,-qtconnectivity,-qtxmlpatterns,-qtremoteobjects,-qtdocgallery,-qtcharts,-qtmultimedia,-qtgraphicaleffects,-qtlottie,-qtactiveqt && \
    ./configure -opensource -confirm-license -nomake examples -nomake tests --silent

ENV QT_WEBGL_WEBSOCKETSERVER ws://0.0.0.0:8081

WORKDIR /root
