#!/bin/bash
echo "root:root" | chpasswd

#for sshd
mkdir /var/run/sshd

exec /usr/bin/supervisord -n


