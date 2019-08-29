#!/bin/bash
PASSWD=$(printenv $PASSWD)
echo "root:"${PASSWD} | chpasswd

exec /usr/bin/supervisord -n
