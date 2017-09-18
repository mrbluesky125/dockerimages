FROM ubuntu:16.04

ENV DEBIAN_FRONTEND noninteractive
ENV QT_VERSION 5.9.1

RUN apt-get update && \
    apt-get upgrade -y --force-yes && \
    apt-get install -y git && \
    apt-get autoclean && apt-get autoremove && rm -rf /var/lib/apt/lists/* 
   
RUN apt-get update && \
    apt-get install -y libxcb-xinerama0-dev && \
    apt-get install -y '^libxcb.*-dev' libx11-xcb-dev libglu1-mesa-dev libxrender-dev libxi-dev && \
    apt-get install -y flex bison gperf libicu-dev libxslt-dev ruby && \
    apt-get install -y libssl-dev libxcursor-dev libxcomposite-dev libxdamage-dev libxrandr-dev libdbus-1-dev libfontconfig1-dev libcap-dev libxtst-dev libpulse-dev libudev-dev libpci-dev libnss3-dev libasound2-dev libxss-dev libegl1-mesa-dev gperf bison && \
    apt-get install -y libasound2-dev libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev && \
    apt-get install -y locate && \
    apt-get install -y libpq-dev postgresql && \
    apt-get autoclean && apt-get autoremove && rm -rf /var/lib/apt/lists/* 

RUN git clone https://code.qt.io/qt/qt5.git && \
    cd qt5 && git checkout tags/v$QT_VERSION && ./init-repository && \
    ./configure -opensource -confirm-license -nomake examples -nomake tests && \
    make -j32 -s && make install -j32 && \
    git submodule foreach --recursive "git clean -dfx" && git clean -dfx
    
ENV PATH /usr/local/Qt-$QT_VERSION/bin:$PATH
ENV LD_LIBRARY_PATH /usr/local/Qt-$QT_VERSION/lib:$LD_LIBRARY_PATH

RUN echo 'export QT_VERSION='QT_VERSION >> /etc/bash.bashrc
RUN echo 'export PATH='$PATH >> /etc/bash.bashrc
RUN echo 'export LD_LIBRARY_PATH='$LD_LIBRARY_PATH >> /etc/bash.bashrc

WORKDIR /root
