FROM xrdpcuda_image

MAINTAINER Timo Zimmermann <zimmermann.emb@googlemail.com>

ENV DEBIAN_FRONTEND noninteractive

#Python + Tensorflow
#RUN apt-get update && \
#    apt-get install -y python-pip python3-dev && \
#    apt-get autoclean && apt-get autoremove && rm -rf /var/lib/apt/lists/* && \

#Anaconda
RUN echo 'export PATH=/opt/conda/bin:$PATH' > /etc/profile.d/conda.sh && \
    wget --quiet https://repo.continuum.io/archive/Anaconda3-4.4.0-Linux-x86_64.sh -O ~/anaconda.sh && \
    /bin/bash ~/anaconda.sh -b -p /opt/conda && \
    rm ~/anaconda.sh

ENV PATH /opt/conda/bin:$PATH

#PIP Python
RUN pip install --upgrade pip  && \  
    pip install tensorflow-gpu

#add ENV variables to .bashrc for remote logins
RUN echo 'export PATH='$PATH >> /etc/bash.bashrc
RUN echo 'export LD_LIBRARY_PATH='$LD_LIBRARY_PATH >> /etc/bash.bashrc
RUN echo 'export PYTHONPATH='$PYTHONPATH\ >> /etc/bash.bashrc

VOLUME ["/root/workspace"]

EXPOSE 3389
EXPOSE 22

WORKDIR /root

CMD ["/usr/local/start.sh"]
