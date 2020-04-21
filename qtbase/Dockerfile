FROM ubuntu:18.04

ENV DEBIAN_FRONTEND noninteractive

#build prerequesits
RUN apt-get update && apt-get upgrade -y --force-yes && \
    apt-get install -y build-essential git python perl && \
    apt-get install -y '^libxcb.*-dev' libxcb-xinerama0-dev libx11-xcb-dev libglu1-mesa-dev libxrender-dev libxi-dev && \
    apt-get install -y flex bison gperf libicu-dev libxslt-dev ruby && \
    apt-get install -y libssl-dev libxcursor-dev libxcomposite-dev libxdamage-dev libxrandr-dev libdbus-1-dev libfontconfig1-dev libcap-dev libxtst-dev libpulse-dev libudev-dev libpci-dev libnss3-dev libasound2-dev libxss-dev libegl1-mesa-dev gperf bison && \
    apt-get install -y libasound2-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev && \
    apt-get install -y locate && \
    apt-get install -y libpq-dev libprotobuf-dev postgresql && \
    apt-get autoclean && apt-get autoremove && rm -rf /var/lib/apt/lists/* 

#python3/aws stuff
RUN apt-get update && \
    apt-get install -y --force-yes python3 python3-pip && \
    apt-get autoclean && apt-get autoremove && rm -rf /var/lib/apt/lists/* && \
    pip3 install boto3

ENV QT_VERSION_TAG 5.14.0

RUN cd /opt && git clone https://code.qt.io/qt/qt5.git && \
    cd qt5 && git checkout tags/v$QT_VERSION_TAG && \ 
    ./init-repository --module-subset=default,-qtwebview,-qtwebengine,-qtwebchannel,-qt3d,-qtcanvas3d,-qtscript,-qtwayland,-qtdatavis3d,-qtpurchasing,-qtlocation,-qtgamepad,-qtdoc,-qtconnectivity,-qtxmlpatterns,-qtremoteobjects,-qtdocgallery,-qtmultimedia,-qtlottie && \
    ./configure -opensource -confirm-license -nomake examples -nomake tests --silent

WORKDIR /root
