FROM xrdpcuda_image

MAINTAINER Timo Zimmermann <zimmermann.emb@googlemail.com>

ENV DEBIAN_FRONTEND noninteractive

#Anaconda
RUN echo 'export PATH=/opt/conda/bin:$PATH' > /etc/profile.d/conda.sh && \
    wget --quiet https://repo.continuum.io/archive/Anaconda3-4.4.0-Linux-x86_64.sh -O ~/anaconda.sh && \
    /bin/bash ~/anaconda.sh -b -p /opt/conda && \
    rm ~/anaconda.sh
ENV PATH /opt/conda/bin:$PATH

#PIP Python
RUN pip install --upgrade pip  && \  
    pip install tqdm && \
    pip install lmdb && \
    pip install tensorflow-gpu && \
    pip install keras

#add ENV variables to .bashrc for remote logins
RUN sed -i '/export /d' /etc/bash.bashrc
RUN echo 'export PATH='$PATH >> /etc/bash.bashrc
RUN echo 'export LD_LIBRARY_PATH='$LD_LIBRARY_PATH >> /etc/bash.bashrc
RUN echo 'export PYTHONPATH='$PYTHONPATH >> /etc/bash.bashrc
RUN echo 'export QT_XKB_CONFIG_ROOT=/usr/share/X11/xkb' >> /etc/bash.bashrc

#create shortcut
RUN touch /root/spyderlauncher.sh && \
    echo 'export PATH='$PATH >> /root/spyderlauncher.sh && \
    echo 'export LD_LIBRARY_PATH='$LD_LIBRARY_PATH >> /root/spyderlauncher.sh && \
    echo 'export QT_XKB_CONFIG_ROOT=/usr/share/X11/xkb' >> /root/spyderlauncher.sh && \
    echo '/opt/conda/bin/spyder --new-instance' >> /root/spyderlauncher.sh && \
    chmod +x /root/spyderlauncher.sh && \
    cp /opt/conda/share/applications/spyder3.desktop /root/Desktop/spyder.desktop  && \
    sed -i '/Exec=spyder3/d' /root/Desktop/spyder.desktop  && \
    echo "Exec=/root/spyderlauncher.sh" >> /root/Desktop/spyder.desktop  && \
    echo "Icon=/opt/conda/share/pixmaps/spyder3.png" >> /root/Desktop/spyder.desktop  && \
    chmod +x /root/Desktop/spyder.desktop
    
#make chromium shortcut working
RUN sed -i '/Exec=chromium-browser/s/$/ --no-sandbox/' /root/Desktop/chromium-browser.desktop && \
    chmod +x /root/Desktop/chromium-browser.desktop

ADD evdev /usr/share/X11/xkb/keycodes/evdev

VOLUME ["/root/workspace"]

EXPOSE 3389
EXPOSE 6006
EXPOSE 22

WORKDIR /root

CMD ["/usr/local/start.sh"]
