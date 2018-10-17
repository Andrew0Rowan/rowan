#!/bin/bash

HDMIStatus=$(cat /sys/class/drm/card0-HDMI-A-1/status)

if [ $HDMIStatus = 'connected' ]; then
        systemctl restart weston
else
        systemctl stop weston
fi
