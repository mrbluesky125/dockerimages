#!/bin/bash
CONTAINER=$CONTAINER
PASSWD_ENV_NAME=${CONTAINER}_PASSWD
PASSWD=$(printenv ${PASSWD_ENV_NAME})
echo "root:"${PASSWD} | chpasswd

sed -i 's/username=ask/username=root/g' /etc/xrdp/xrdp.ini
sed -i '/\[globals\]/a autorun=sesman-Xvnc' /etc/xrdp/xrdp.ini
sed -i '/\[globals\]/a hidelogwindow=1' /etc/xrdp/xrdp.ini
sed -i '/switch_window_key/d' /etc/xdg/xfce4/xfconf/xfce-perchannel-xml/xfce4-keyboard-shortcuts.xml
rm /etc/xdg/autostart/xscreensaver.desktop

exec /usr/bin/supervisord -n
