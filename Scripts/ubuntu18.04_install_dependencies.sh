#!/bin/bash

add-apt-repository --yes ppa:beineri/opt-qt-5.11.0-bionic
apt update
apt install -y cmake mesa-common-dev libx11-dev libepoxy-dev libsoil-dev libassimp-dev libfreetype6-dev libopenal-dev libogg-dev libvorbis-dev libbox2d-dev libbullet-dev qt511base qtbase5-dev libsdl2-dev