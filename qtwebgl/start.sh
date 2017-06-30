#!/bin/bash
echo "root:root" | chpasswd

exec /usr/bin/supervisord -n


