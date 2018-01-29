FROM ubuntu:16.04

ENV DEBIAN_FRONTEND noninteractive
ENV QT_VERSION_TAG 5.10.0

RUN apt-get update && apt-get upgrade -y --force-yes && \
    apt-get install -y build-essential git python perl && \
    apt-get install -y '^libxcb.*-dev' libxcb-xinerama0-dev libx11-xcb-dev libglu1-mesa-dev libxrender-dev libxi-dev && \
    apt-get install -y flex bison gperf libicu-dev libxslt-dev ruby && \
    apt-get install -y libssl-dev libxcursor-dev libxcomposite-dev libxdamage-dev libxrandr-dev libdbus-1-dev libfontconfig1-dev libcap-dev libxtst-dev libpulse-dev libudev-dev libpci-dev libnss3-dev libasound2-dev libxss-dev libegl1-mesa-dev gperf bison && \
    apt-get install -y libasound2-dev libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev && \
    apt-get install -y locate && \
    apt-get install -y libpq-dev libprotobuf-dev postgresql && \
    apt-get autoclean && apt-get autoremove && rm -rf /var/lib/apt/lists/* 

RUN cd opt && git clone https://code.qt.io/qt/qt5.git && \
    cd qt5 && git checkout tags/v$QT_VERSION_TAG && \ 
    ./init-repository --module-subset=default,-qtwebkit,-qtwebkit-examples,-qtwebengine && \
    ./configure -opensource -confirm-license -nomake examples -nomake tests --silent && \
    make -j16 -s && make install && \
    git submodule foreach --recursive "git clean -dfx" && git clean -dfx
    
ENV PATH /usr/local/Qt-$QT_VERSION_TAG/bin:$PATH
ENV LD_LIBRARY_PATH /usr/local/Qt-$QT_VERSION_TAG/lib:$LD_LIBRARY_PATH

RUN echo 'export QT_VERSION='QT_VERSION_TAG >> /etc/bash.bashrc
RUN echo 'export PATH='$PATH >> /etc/bash.bashrc
RUN echo 'export LD_LIBRARY_PATH='$LD_LIBRARY_PATH >> /etc/bash.bashrc

WORKDIR /root
