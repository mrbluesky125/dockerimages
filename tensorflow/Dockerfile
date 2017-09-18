FROM xrdpcuda_image

MAINTAINER Timo Zimmermann <zimmermann.emb@googlemail.com>

ENV DEBIAN_FRONTEND noninteractive

#Python
RUN apt-get update && \
    apt-get install -y python && \
    apt-get autoclean && apt-get autoremove && rm -rf /var/lib/apt/lists/*

#add ENV variables to .bashrc for remote logins
RUN sed -i '/export /d' /etc/bash.bashrc
RUN echo 'export PATH='$PATH >> /etc/bash.bashrc
RUN echo 'export LD_LIBRARY_PATH='$LD_LIBRARY_PATH >> /etc/bash.bashrc
RUN echo 'export PYTHONPATH='$PYTHONPATH\ >> /etc/bash.bashrc

VOLUME ["/root/workspace"]

EXPOSE 3389
EXPOSE 22

WORKDIR /root

CMD ["/usr/local/start.sh"]
