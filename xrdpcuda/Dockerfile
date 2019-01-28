ARG cuda_version=10.0
ARG cudnn_version=7
FROM nvidia/cuda:${cuda_version}-cudnn${cudnn_version}-devel-ubuntu18.04

MAINTAINER Timo Zimmermann <zimmermann.emb@googlemail.com>

ENV DEBIAN_FRONTEND noninteractive

#Remote desktop XRDP
RUN apt-get update && \
    apt-get install -y --force-yes xrdp xvfb xfce4 xfce4-terminal gnome-icon-theme-full tango-icon-theme leafpad evince ristretto vlc chromium-browser && \
    apt-get autoclean && apt-get autoremove && rm -rf /var/lib/apt/lists/* && \
    sed -i '/TerminalServerUsers/d' /etc/xrdp/sesman.ini && \
    sed -i '/TerminalServerAdmins/d' /etc/xrdp/sesman.ini && \
    sed -i 's/KillDisconnected=0/KillDisconnected=1/g' /etc/xrdp/sesman.ini && \
    sed -i 's/DisconnectedTimeLimit=0/DisconnectedTimeLimit=10/g' /etc/xrdp/sesman.ini && \
    sed -i '/switch_window_key/d' /etc/xdg/xfce4/xfconf/xfce-perchannel-xml/xfce4-keyboard-shortcuts.xml && \
    rm /etc/xdg/autostart/xscreensaver.desktop && \
    mkdir /root/Desktop/ && \
	cp /usr/share/applications/chromium-browser.desktop /root/Desktop/ && \
    xrdp-keygen xrdp auto

RUN echo xfce4-session >~/.xsession

#Supervisor und SSH
RUN apt-get update && \
    apt-get upgrade -y --force-yes && \
    apt-get install -y supervisor wget nano htop cmake git strace openssh-server && \
    apt-get autoclean && apt-get autoremove && rm -rf /var/lib/apt/lists/* && \
    sed -i 's/PermitRootLogin without-password/PermitRootLogin yes/' /etc/ssh/sshd_config && \
    mkdir /var/run/sshd

ADD xrdp.ini /etc/xrdp/xrdp.ini
ADD supervisor.conf /etc/supervisor/conf.d/supervisor.conf

#add ENV variables to .bashrc for remote logins
RUN echo 'export PATH='$PATH >> /etc/bash.bashrc
RUN echo 'export LD_LIBRARY_PATH='$LD_LIBRARY_PATH >> /etc/bash.bashrc
RUN echo 'export PYTHONPATH='$PYTHONPATH\ >> /etc/bash.bashrc

VOLUME ["/root/workspace"]

EXPOSE 3389
EXPOSE 22

WORKDIR /root

ADD start.sh /usr/local/start.sh
RUN chmod 777 /usr/local/start.sh && chmod +x /usr/local/start.sh

CMD ["/usr/local/start.sh"]
