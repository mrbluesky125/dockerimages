#!/bin/bash
CONTAINER=$CONTAINER
PASSWD_ENV_NAME=${CONTAINER}_PASSWD
PASSWD=$(printenv ${PASSWD_ENV_NAME})
echo "root:"${PASSWD} | chpasswd

#delete all other xrdp configs
#sed -i '/\[xrdp2\]/Q' /etc/xrdp/xrdp.ini
#sed -i '/\[xrdp2\]/d' /etc/xrdp/xrdp.ini

#sed -i 's/username=ask/username=root/g' /etc/xrdp/xrdp.ini
#sed -i 's/port=-1/port=ask-1/g' /etc/xrdp/xrdp.ini
#sed -i '/\[globals\]/a autorun=sesman-Xvnc' /etc/xrdp/xrdp.ini
#sed -i '/\[globals\]/a hidelogwindow=1' /etc/xrdp/xrdp.ini

#sesman config
sed -i 's/KillDisconnected=0/KillDisconnected=1/g' /etc/xrdp/sesman.ini
sed -i 's/DisconnectedTimeLimit=0/DisconnectedTimeLimit=10/g' /etc/xrdp/sesman.ini
sed -i 's/MaxSessions=10/MaxSessions=50/g' /etc/xrdp/sesman.ini

#fix tab completion
sed -i '/switch_window_key/d' /etc/xdg/xfce4/xfconf/xfce-perchannel-xml/xfce4-keyboard-shortcuts.xml

#create desktop shortcut
sed -i '/Exec=spyder/d' /root/Desktop/spyder.desktop
echo "Exec=/opt/conda/bin/spyder --new-instance" >> /root/Desktop/spyder.desktop
echo "Icon=/opt/conda/share/pixmaps/spyder.png" >> /root/Desktop/spyder.desktop
chmod +x /root/Desktop/spyder.desktop

#remove screensaver daemon
rm /etc/xdg/autostart/xscreensaver.desktop

exec /usr/bin/supervisord -n
