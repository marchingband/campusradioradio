#!/bin/bash 

if [ "$1" == "compile" ]; then 
    echo "compiling"
    cd .. && \
    arduino-cli compile -e --build-property build.code_debug=3 --fqbn esp32:esp32:esp32wrover campusradioradio && \
    echo "done"
elif [ "$1" == "flash" ]; then 
    cd .. && \
    echo "uploading" && \
    arduino-cli upload -p /dev/cu.usbserial-A50285BI --fqbn esp32:esp32:esp32wrover campusradioradio && \
    echo "done"
elif [ "$1" == "code" ]; then 
    cd .. && \
    arduino-cli compile -e --build-property build.code_debug=3 --fqbn esp32:esp32:esp32wrover campusradioradio && \
    echo "done compilation, uploading" && \
    arduino-cli upload -p /dev/cu.usbserial-A50285BI --fqbn esp32:esp32:esp32wrover campusradioradio && \
    echo "done"
else
    echo "please use one of flags : code, compile, or flash"
fi