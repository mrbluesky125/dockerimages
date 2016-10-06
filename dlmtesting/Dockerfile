FROM nvidia/cuda:7.5-cudnn5-devel-ubuntu14.04

MAINTAINER Timo Zimmermann <zimmermann.emb@googlemail.com>

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && \
    apt-get upgrade -y --force-yes && \
    apt-get install -y supervisor wget nano htop cmake git strace && \
    apt-get autoclean && apt-get autoremove && rm -rf /var/lib/apt/lists/*

RUN apt-get update && \
    apt-get install -y --force-yes xrdp xvfb xfce4 xfce4-terminal gnome-icon-theme-full tango-icon-theme leafpad && \
    apt-get autoclean && apt-get autoremove && rm -rf /var/lib/apt/lists/* && \
    sed -i '/TerminalServerUsers/d' /etc/xrdp/sesman.ini && \
    sed -i '/TerminalServerAdmins/d' /etc/xrdp/sesman.ini && \
    xrdp-keygen xrdp auto

RUN echo xfce4-session >~/.xsession

RUN echo 'export PATH=/opt/conda/bin:$PATH' > /etc/profile.d/conda.sh && \
    wget --quiet https://repo.continuum.io/archive/Anaconda2-4.1.1-Linux-x86_64.sh -O ~/anaconda.sh && \
    /bin/bash ~/anaconda.sh -b -p /opt/conda && \
    rm ~/anaconda.sh

ENV PATH /opt/conda/bin:$PATH

    # HDF5 support
RUN conda install h5py && \
    # imbalanced dataset
    conda install -c glemaitre imbalanced-learn

# Install dependencies for Caffe
RUN apt-get update && apt-get install -y libopenblas-dev libatlas-base-dev libboost-all-dev libgflags-dev libgoogle-glog-dev libhdf5-serial-dev libleveldb-dev liblmdb-dev libopencv-dev libprotobuf-dev libsnappy-dev protobuf-compiler && \
    apt-get clean && apt-get autoremove && rm -rf /var/lib/apt/lists/*

# Install Caffe
RUN git clone -b master --depth 1 https://github.com/BVLC/caffe.git /opt/caffe && \
    cd /opt/caffe && \
    cat python/requirements.txt | xargs -n1 pip install && \
    mkdir build && cd build && \
#    cmake -DUSE_CUDNN=1 -DCUDA_gpu_detect_output="6.2 6.2" .. && \
    cmake -DUSE_CUDNN=1 .. && \
    make -j"$(nproc)"

ADD Makefile.config /opt/caffe/Makefile.config
RUN cd /opt/caffe && make -j"$(nproc)" && make -j"$(nproc)" pycaffe

ENV PATH /opt/caffe/build/tools:/opt/caffe/python:$PATH

RUN echo "/opt/caffe/build/lib" >> /etc/ld.so.conf.d/caffe.conf && ldconfig
RUN ln /dev/null /dev/raw1394

ADD xrdp.conf /etc/supervisor/conf.d/xrdp.conf

#setup root as default user
RUN echo "root:root" | chpasswd
RUN sed -i 's/username=ask/username=root/g' /etc/xrdp/xrdp.ini && \
    sed -i 's/password=ask/password=root/g' /etc/xrdp/xrdp.ini && \
    sed -i '/\[globals\]/a autorun=sesman-Xvnc' /etc/xrdp/xrdp.ini && \
    sed -i '/\[globals\]/a hidelogwindow=1' /etc/xrdp/xrdp.ini

#add ENV variables to .bashrc for remote logins
RUN echo 'export PATH='$PATH >> ~/.bashrc
RUN echo 'export LD_LIBRARY_PATH='$LD_LIBRARY_PATH >> ~/.bashrc
RUN echo 'export PYTHONPATH=/opt/caffe/python:'$PYTHONPATH >> ~/.bashrc

#fix tab completion
#RUN sed -i '/switch_window_key/d' /root/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-keyboard-shortcuts.xml

VOLUME ["/root/workspace"]
VOLUME ["/root/share"]

EXPOSE 3389

CMD ["/usr/bin/supervisord", "-n"]
