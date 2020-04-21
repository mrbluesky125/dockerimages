#!/bin/bash
cd /home/bitbake/yocto/build/tmp/deploy/images/jetson-nano-qspi-sd/
rm -r ./output
mkdir -p ./output
unzip 'qt5-nano-image-jetson-nano-qspi-sd-*.tegraflash.zip' -d ./output

echo "BOARDID=3448 \nFAB=301 \nBOARDSKU=0000 \nBOARDREV=a02" >> ./output/flashvars
cd ./output
./dosdcard.sh

mv *.sdcard ../
rm -r ./output
