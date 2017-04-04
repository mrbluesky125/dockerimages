#!/bin/bash
CONTAINER=$CONTAINER
PASSWD_ENV_NAME=${CONTAINER}_PASSWD
PASSWD=$(printenv ${PASSWD_ENV_NAME})
echo "root:root" | chpasswd

#allow root ssh access
sed -i 's/PermitRootLogin without-password/PermitRootLogin yes/' /etc/ssh/sshd_config

#sesman config
sed -i 's/KillDisconnected=0/KillDisconnected=1/g' /etc/xrdp/sesman.ini
sed -i 's/DisconnectedTimeLimit=0/DisconnectedTimeLimit=10/g' /etc/xrdp/sesman.ini
sed -i 's/MaxSessions=10/MaxSessions=50/g' /etc/xrdp/sesman.ini

#fix tab completion
sed -i '/switch_window_key/d' /etc/xdg/xfce4/xfconf/xfce-perchannel-xml/xfce4-keyboard-shortcuts.xml

#create spyderlauncher.sh
#touch /root/spyderlauncher.sh
#echo 'export PATH='$PATH >> /root/spyderlauncher.sh
#echo 'export LD_LIBRARY_PATH='$LD_LIBRARY_PATH >> /root/spyderlauncher.sh
#echo 'export PYTHONPATH=/opt/caffe/python:' >> /root/spyderlauncher.sh
#echo '/opt/conda/bin/spyder --new-instance' >> /root/spyderlauncher.sh
#chmod +x /root/spyderlauncher.sh

#create desktop shortcut
#mkdir /root/Desktop && cp /opt/conda/share/applications/spyder.desktop /root/Desktop/
#sed -i '/Exec=spyder/d' /root/Desktop/spyder.desktop
#echo "Exec=/root/spyderlauncher.sh" >> /root/Desktop/spyder.desktop
#echo "Icon=/opt/conda/share/pixmaps/spyder.png" >> /root/Desktop/spyder.desktop
#chmod +x /root/Desktop/spyder.desktop

remove screensaver daemon
rm /etc/xdg/autostart/xscreensaver.desktop

exec sh /qt5/qtwebglplugin/examples/clocks/clocks --platform webgl

exec /usr/bin/supervisord -n


