#/bin/sh
. /home/bitbake/yocto/oe-init-build-env
if [ "$ISBITBAKECMD" = 1 ] 
    then
        bitbake $BITBAKECMD;
    else
        $BITBAKECMD;
fi;