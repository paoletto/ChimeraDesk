#!/bin/bash -x
FFMPEGPORT=12345
DISPLAY=:0
OUTPUTW=1024
OUTPUTH=768

# try to guess the screen resolution for ffmpeg
Xres=$(xrandr -d $DISPLAY | grep '*' | uniq | awk '{print $1}' | cut -d 'x' -f1)
Yres=$(xrandr -d $DISPLAY | grep '*' | uniq | awk '{print $1}' | cut -d 'x' -f2)
AR=$(echo "${Xres} / ${Yres}" |bc -l |colrm 10)

FFMPEG="ffmpeg"
#$FFMPEG  -init_hw_device qsv -hwaccel qsv -f x11grab -draw_mouse 0  -s ${Xres}x${Yres} -framerate 30 -i :0  -vf "scale=${OUTPUTW}:${OUTPUTH},mpdecimate=hi=1:lo=1:frac=1:max=8"  -c:v hevc_qsv  -load_plugin hevc_hw -preset medium -global_quality 22 -f mpegts-bufsize 512k -f mpegts "tcp://127.0.0.1:${FFMPEGPORT}\?listen"
#$FFMPEG  -init_hw_device qsv -hwaccel qsv -f x11grab -draw_mouse 0  -s ${Xres}x${Yres} -framerate 30 -i :0  -vf "scale=${OUTPUTW}:${OUTPUTH}"  -c:v hevc_qsv  -load_plugin hevc_hw -preset medium -global_quality 22 -f mpegts  /tmp/foo.mpegts
#${FFMPEG} -f x11grab -draw_mouse 0 -s ${Xres}x${Yres} -framerate 30 -i ${DISPLAY} -vf \"scale=854:480\" -c:v libx264 -preset medium -profile high -pix_fmt yuv420p -tune zerolatency -b:v 500K -minrate 500K -maxrate 500K -bufsize 512k -f mpegts tcp://127.0.0.1:${FFMPEGPORT}\?listen
${FFMPEG} -f x11grab -draw_mouse 1 -s ${Xres}x${Yres} -framerate 25 -i ${DISPLAY} -vf "scale=${OUTPUTW}:${OUTPUTH}" -c:v h264_qsv -preset medium -profile baseline -aspect ${AR} -pix_fmt yuv420p -global_quality 30 -look_ahead_depth 0 -max_dec_frame_buffering 0 -bufsize 128k -f mpegts "tcp://127.0.0.1:12345?listen"
