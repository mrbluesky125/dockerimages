#/bin/bash
. /scripts/log.sh

info "Start Yocto-Buildenv"
# info "Check for Firstrun"

#if [ ! -e "/root/yocto/.firstrun" ]
#then 
#    info "First Run"
#    rsync -vua --delete-after $PWD/poky/* /root/yocto
#    ls -al /root/yocto/
#    touch /root/yocto/.firstrun
#else
#    info "Not first run"
#fi

info "Copy SSH-Key"
cp -r /home/bitbake/ssh /home/bitbake/.ssh

info "Start Bitbake"
chown -R bitbake:bitbake /home/bitbake/yocto/build
chown -R bitbake:bitbake /home/bitbake/.ssh
su -m bitbake -c /scripts/run_bitbake.sh
