#!/bin/bash
CONTAINER=$CONTAINER
PASSWD_ENV_NAME=${CONTAINER}_PASSWD
PASSWD=$(printenv ${PASSWD_ENV_NAME})
echo "root:"${PASSWD} | chpasswd

sed -i 's/username=ask/username=root/g' /etc/xrdp/xrdp.ini
sed -i '/\[globals\]/a autorun=sesman-Xvnc' /etc/xrdp/xrdp.ini
sed -i '/\[globals\]/a hidelogwindow=1' /etc/xrdp/xrdp.ini

exec /usr/bin/supervisord -n

sed -i '/switch_window_key/d' /root/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-keyboard-shortcuts.xml
