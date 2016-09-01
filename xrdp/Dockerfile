#FROM ubuntu:xenial
FROM nvidia/cuda:7.5-cudnn5-devel-ubuntu14.04

MAINTAINER Hans Meiser <hans@meiser.com>

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update -y && \
    apt-get upgrade -y && \
    apt-get install -y supervisor && \
    apt-get autoclean && apt-get autoremove && \
    rm -rf /var/lib/apt/lists/*

RUN apt-get update -y && \
    apt-get install -y xrdp xfce4 xfce4-terminal gnome-icon-theme-full tango-icon-theme && \
    apt-get autoclean && apt-get autoremove && \
    rm -rf /var/lib/apt/lists/* && \
    useradd -ms /bin/bash desktop && \
    sed -i '/TerminalServerUsers/d' /etc/xrdp/sesman.ini && \
    sed -i '/TerminalServerAdmins/d' /etc/xrdp/sesman.ini && \
    xrdp-keygen xrdp auto && \
    echo "desktop:desktop" | chpasswd

RUN echo xfce4-session >~/.xsession

RUN apt-get update -y && \
    apt-get install -y python-pip python-qt4 python-sphinx && \
    pip install spyder

ADD xrdp.conf /etc/supervisor/conf.d/xrdp.conf

EXPOSE 3389

CMD ["/usr/bin/supervisord", "-n"]
