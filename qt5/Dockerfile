ARG cuda_version=11.0
ARG cudnn_version=8
ARG cuda_package=runtime
ARG ubuntu_version=18.04

FROM nvidia/cuda:${cuda_version}-cudnn${cudnn_version}-${cuda_package}-ubuntu${ubuntu_version}

ENV qt_version 5.15.1
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get upgrade -y --force-yes && \
    apt-get install -y build-essential cmake git python python3 python3-dev perl locate wget checkinstall && \
    apt-get install -y '^libxcb.*-dev' libxcb-xinerama0-dev libx11-xcb-dev libglu1-mesa-dev libxrender-dev libxi-dev && \
    apt-get install -y flex bison gperf libicu-dev libxslt-dev ruby && \
    apt-get install -y libssl-dev libxcursor-dev libxcomposite-dev libxdamage-dev libxrandr-dev libdbus-1-dev libfontconfig1-dev libcap-dev libxtst-dev libpulse-dev libudev-dev libpci-dev libnss3-dev libasound2-dev libxss-dev libegl1-mesa-dev gperf bison && \
    apt-get install -y libasound2-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev && \
    apt-get install -y libpq-dev libprotobuf-dev postgresql && \
#	apt-get install -y gcc-10 g++-10 llvm-10 virtualenvwrapper clang-10 libclang-10-dev libxslt-dev mesa-common-dev libgl1-mesa-glx libglib2.0-0 && \
    apt-get install -y gcc-6 g++-6 llvm-6.0 virtualenvwrapper clang-6.0 libclang-6.0-dev libxslt-dev mesa-common-dev libgl1-mesa-glx libglib2.0-0 && \
    apt-get install -y libwayland-dev libwayland-egl1-mesa libwayland-server0 libgles2-mesa-dev libxkbcommon-dev && \
    apt-get autoclean && apt-get autoremove && rm -rf /var/lib/apt/lists/* 

RUN cd opt && git clone https://code.qt.io/qt/qt5.git && \
    cd qt5 && git checkout tags/v$qt_version && \ 
    ./init-repository --module-subset=default,-qtwebkit,-qtwebkit-examples,-qtwebengine,-qtwebview,-qtwebchannel && \
    ./configure -opensource -confirm-license -nomake examples -nomake tests --silent && \
    make -j$(grep -c ^processor /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu) && make install && \
    git submodule foreach --recursive "git clean -dfx" && git clean -dfx && \
    cd .. && rm -Rf qt5

RUN locale-gen en_US.UTF-8 && locale -a

ENV PATH /usr/local/Qt-$qt_version/bin:$PATH
ENV LD_LIBRARY_PATH /usr/local/Qt-$qt_version/lib:$LD_LIBRARY_PATH

RUN echo 'export QT_VERSION='$qt_version >> /etc/bash.bashrc
RUN echo 'export PATH='$PATH >> /etc/bash.bashrc
RUN echo 'export LD_LIBRARY_PATH='$LD_LIBRARY_PATH >> /etc/bash.bashrc

WORKDIR /root
