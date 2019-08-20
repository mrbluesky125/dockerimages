FROM ubuntu:18.04

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get upgrade -y --force-yes && \
    apt-get install -y build-essential git python perl && \
    apt-get install -y '^libxcb.*-dev' libxcb-xinerama0-dev libx11-xcb-dev libglu1-mesa-dev libxrender-dev libxi-dev && \
    apt-get install -y flex bison gperf libicu-dev libxslt-dev ruby && \
    apt-get install -y libssl-dev libxcursor-dev libxcomposite-dev libxdamage-dev libxrandr-dev libdbus-1-dev libfontconfig1-dev libcap-dev libxtst-dev libpulse-dev libudev-dev libpci-dev libnss3-dev libasound2-dev libxss-dev libegl1-mesa-dev gperf bison && \
    apt-get install -y libasound2-dev libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev && \
    apt-get install -y locate && \
    apt-get install -y libpq-dev libprotobuf-dev postgresql && \
    apt-get autoclean && apt-get autoremove && rm -rf /var/lib/apt/lists/* 

WORKDIR /root
