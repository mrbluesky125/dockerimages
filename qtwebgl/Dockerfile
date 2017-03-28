FROM ubuntu:16.04

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && \
    apt-get upgrade -y --force-yes && \
    apt-get install -y supervisor wget nano htop cmake git strace openssh-server&& \
    apt-get autoclean && apt-get autoremove && rm -rf /var/lib/apt/lists/*
    
RUN sed -i 's/PermitRootLogin without-password/PermitRootLogin yes/' /etc/ssh/sshd_config

RUN apt-get update && \
    apt-get install -y --force-yes xrdp xvfb xfce4 xfce4-terminal gnome-icon-theme-full tango-icon-theme leafpad netsurf && \
    apt-get autoclean && apt-get autoremove && rm -rf /var/lib/apt/lists/* && \
    sed -i '/TerminalServerUsers/d' /etc/xrdp/sesman.ini && \
    sed -i '/TerminalServerAdmins/d' /etc/xrdp/sesman.ini && \
    xrdp-keygen xrdp auto
    
RUN echo xfce4-session >~/.xsession

RUN apt-get update && \
    apt-get build-dep qt5-default && \
    apt-get install libxcb-xinerama0-dev && \
    apt-get install '^libxcb.*-dev' libx11-xcb-dev libglu1-mesa-dev libxrender-dev libxi-dev && \
    apt-get install flex bison gperf libicu-dev libxslt-dev ruby && \
    apt-get install libssl-dev libxcursor-dev libxcomposite-dev libxdamage-dev libxrandr-dev libdbus-1-dev libfontconfig1-dev libcap-dev libxtst-dev libpulse-dev libudev-dev libpci-dev libnss3-dev libasound2-dev libxss-dev libegl1-mesa-dev gperf bison && \
    apt-get install libasound2-dev libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev && \
     
RUN apt-get update && \
    git clone https://code.qt.io/qt/qt5.git && \
    cd qt5 && \
    git checkout 5.9 && \
    ./init-repository && \
    ./configure -opensource -confirm-license && \
    make -j32 && \
    make install -j32 
    
RUN echo 'export PATH='$PATH >> /etc/bash.bashrc
RUN echo 'export LD_LIBRARY_PATH='$LD_LIBRARY_PATH >> /etc/bash.bashrc
    
    
EXPOSE 3389
EXPOSE 22
WORKDIR /root

ADD xrdp.ini /etc/xrdp/xrdp.ini

RUN sudo service ssh start
    

