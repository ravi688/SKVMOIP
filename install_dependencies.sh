#!/bin/bash


# Platform detection
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" || "$OSTYPE" == "mingw"* ]]; then
        PLATFORM="MINGW"
else
        PLATFORM="LINUX"
        if [ "$EUID" -ne 0 ]; then
                echo "This script must be run as root. Please use sudo."
                exit -1
        fi

        is_in_docker() {
                [ -f "/.dockerenv" ] || grep -qa docker /proc/1/cgroup
        }

        # Detect if we are running on docker container
        if is_in_docker || [ -z "${SUDO_USER-}" ]; then
                NO_ROOT=""
        else
                NO_ROOT="sudo -u $SUDO_USER"
        fi
fi


$NO_ROOT meson wrap install vulkan-headers

if [ "$PLATFORM" == "MINGW" ]; then
	pacman -S mingw-w64-x86_64-gtk3
else
	apt-get install libgtk-3-dev
fi
