#!/bin/sh -xe

# ssh target info
REMOTEHOST=logger@mail.plasmatronics.com.au
REMOTEPORT="1085"

# clean and build application
make clean 
make all

# upload new firmware to target
ssh -p $REMOTEPORT $REMOTEHOST mkdir -p bin
rsync -a -e "ssh -p $REMOTEPORT" bin/buslogger $REMOTEHOST:bin
# log on to target and start buslogger
ssh -p $REMOTEPORT $REMOTEHOST /bin/bash <<'EOT'
# set path to log
LOGPATH="/home/"$USER"/log"
# kill any existing instance of logger
pkill buslogger
# make sure logging directory exists
mkdir -p $LOGPATH
# start logger
nohup /home/"$USER"/bin/buslogger $LOGPATH </dev/null >/dev/null 2>&1 &
EOT




