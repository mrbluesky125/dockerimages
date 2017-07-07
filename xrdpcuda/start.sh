#!/bin/bash
CONTAINER=$CONTAINER
PASSWD_ENV_NAME=${CONTAINER}_PASSWD
PASSWD=$(printenv ${PASSWD_ENV_NAME})
echo "root:"${PASSWD} | chpasswd

exec /usr/bin/supervisord -n
