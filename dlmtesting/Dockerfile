#FROM ubuntu:xenial
FROM nvidia/cuda:7.5-cudnn5-devel-ubuntu14.04

MAINTAINER Timo Zimmermann <zimmermann.emb@googlemail.com>

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update -y && \
    apt-get upgrade -y && \
    apt-get install -y supervisor && \
    apt-get autoclean && apt-get autoremove && \
    rm -rf /var/lib/apt/lists/*

#XRDP Installation
RUN apt-get update -y && \
    apt-get install -y xrdp xfce4 xfce4-terminal gnome-icon-theme-full tango-icon-theme && \
    apt-get autoclean && apt-get autoremove && \
    rm -rf /var/lib/apt/lists/* && \
    useradd -ms /bin/bash desktop && \
    sed -i '/TerminalServerUsers/d' /etc/xrdp/sesman.ini && \
    sed -i '/TerminalServerAdmins/d' /etc/xrdp/sesman.ini && \
    xrdp-keygen xrdp auto && \
    echo "desktop:desktop" | chpasswd

ADD xrdp.conf /etc/supervisor/conf.d/xrdp.conf
	
RUN echo xfce4-session >~/.xsession

#Tools Installation
RUN apt-get update -y && \
    apt-get install -y python-pip python-qt4 python-sphinx && \
    apt-get install -y htop wget nano cmake strace

RUN echo 'export PATH=/opt/conda/bin:$PATH' > /etc/profile.d/conda.sh && \
    wget --quiet https://repo.continuum.io/archive/Anaconda2-4.1.1-Linux-x86_64.sh -O ~/anaconda.sh && \
    /bin/bash ~/anaconda.sh -b -p /opt/conda && \
    rm ~/anaconda.sh

RUN echo 'export PATH=/opt/conda/bin:$PATH' >> /home/desktop/.bashrc

ENV PATH /opt/conda/bin:$PATH

RUN apt-get update && \
    apt-get install -y libopenblas-dev && \
    apt-get install -y git && \
    # tensorflow
    wget https://storage.googleapis.com/tensorflow/linux/gpu/tensorflow-0.10.0-cp27-none-linux_x86_64.whl && \
    pip install tensorflow-0.10.0-cp27-none-linux_x86_64.whl  && \
    #xgboost
    cd /usr/local/src && mkdir xgboost && cd xgboost && \
    git clone --depth 1 --recursive https://github.com/dmlc/xgboost.git && cd xgboost && \
    make && cd python-package && python setup.py install && \
    #lasagne
    cd /usr/local/src && mkdir Lasagne && cd Lasagne && \
    git clone --depth 1 https://github.com/Lasagne/Lasagne.git && cd Lasagne && \
    pip install -r requirements.txt && python setup.py install && \
    #keras
    cd /usr/local/src && mkdir keras && cd keras && \
    git clone --depth 1 https://github.com/fchollet/keras.git && \
    cd keras && python setup.py install && \
    # Dev branch of Theano
    pip install git+git://github.com/Theano/Theano.git --upgrade --no-deps && \
    # put theano compiledir inside /tmp (it needs to be in writable dir)
    printf "[global]\nbase_compiledir = /tmp/.theano\n" > /.theanorc && \
    cd /usr/local/src &&  git clone --depth 1 https://github.com/pybrain/pybrain && \
    cd pybrain && python setup.py install && \
    # Base ATLAS
    apt-get install -y libatlas-base-dev && \
    # Keras setup
    # Keras likes to add a config file in a custom directory when it's
    # first imported. This doesn't work with our read-only filesystem, so we
    # have it done now
    python -c "from keras.models import Sequential"  && \
    # Switch to TF backend
    sed -i 's/theano/tensorflow/' /root/.keras/keras.json  && \
    # Re-run it to flush any more disk writes
    python -c "from keras.models import Sequential; from keras import backend; print(backend._BACKEND)" && \
    # Keras reverts to /tmp from ~ when it detects a read-only file system
    mkdir -p /tmp/.keras && cp /root/.keras/keras.json /tmp/.keras && \
    # HDF5 support
    conda install h5py && \
    # imbalanced dataset
    conda install -c glemaitre imbalanced-learn

    # tflean. Deep learning library featuring a higher-level API for TensorFlow. http://tflearn.org
RUN pip install git+https://github.com/tflearn/tflearn.git && \
    # clean up pip cache
    rm -rf /root/.cache/pip/*

# Install dependencies for Caffe
RUN apt-get update && apt-get install -y \
		libboost-all-dev \
		libgflags-dev \
		libgoogle-glog-dev \
		libhdf5-serial-dev \
		libleveldb-dev \
		liblmdb-dev \
		libopencv-dev \
		libprotobuf-dev \
		libsnappy-dev \
		protobuf-compiler \
		&& \
	apt-get clean && \
	apt-get autoremove && \
	rm -rf /var/lib/apt/lists/*

# Install Caffe 
RUN git clone -b master --depth 1 https://github.com/BVLC/caffe.git /root/caffe && \
    cd /root/caffe && \
    cat python/requirements.txt | xargs -n1 pip install && \
    mkdir build && cd build && \
    cmake -DUSE_CUDNN=1 -DBLAS=Open .. && \
    make -j"$(nproc)" all && \
    make install

VOLUME ["/home/desktop/persistent"]

EXPOSE 3389

CMD ["/usr/bin/supervisord", "-n"]
