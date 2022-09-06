#!/bin/bash -x

env

FFMPEGPORT=12345
DISPLAY=:1001
DISPLAY=:0

# try to guess the screen resolution for ffmpeg
Xres=$(xrandr -d $DISPLAY | grep '*' | uniq | awk '{print $1}' | cut -d 'x' -f1)
Yres=$(xrandr -d $DISPLAY | grep '*' | uniq | awk '{print $1}' | cut -d 'x' -f2)

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
# Launch KVM client process and store its PID for later termination
if [[ -z $APPDIR ]]; then
    SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
    /usr/bin/python3 $SCRIPTPATH/client/src/client.py &
else
    /usr/bin/python3 $APPDIR/usr/bin/client.py &
fi
kvmpid=$!

# Check whether the KVM process is running, terminate if not.
sleep 1 && kill -0 $kvmpid
kvm_error=$?
if [[ $kvm_error -eq 0 ]]; then
    #notify-send "Screen sharing is starting..."
    echo "Screen sharing is starting..."
else
    #notify-send "Screen sharing could not start"
    echo "Screen sharing could not start"
    exit 1
fi

# If no ffmpeg config file is found, generate a default one
if [[ ! -f ~/.config/streamScreen.sh ]]; then
    cp ${SCRIPTPATH}/streamScreen.sh ~/.config/streamScreen.sh
    #echo "echo \$BASHPID > /tmp/ffmpeg.pid; ffmpeg -f x11grab -draw_mouse 0 -s ${Xres}x${Yres} -framerate 30 -i ${DISPLAY} -vf \"scale=854:480\" -c:v libx264 -preset medium -profile high -pix_fmt yuv420p -tune zerolatency -b:v 500K -minrate 500K -maxrate 500K -bufsize 512k -f mpegts tcp://127.0.0.1:${FFMPEGPORT}\?listen" > ~/.config/hprdpvideo.sh
fi

# Launch ffmpeg screen sharing
bash ~/.config/streamScreen.sh

# Terminate KVM process when ffmpeg stops and feedback the user
kill $kvmpid
