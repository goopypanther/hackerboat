#!/bin/bash

# Set up network
sudo cp /etc/network/interfaces /etc/network/interfaces.bak;
sudo sh -c 'sed s/#connmanctl/connmanctl/ /etc/network/interfaces.bak > /etc/network/interfaces';

# Install packages
sudo apt-get update;
sudo apt-get upgrade;
sudo apt-get install samba gpsd gpsd-clients sqlite3 libsqlite3-dev;

# Group creation
sudo groupadd gpio
sudo usermod -aG gpio debian

# GCC-6 installation
sudo cp /etc/apt/sources.list /etc/apt/sources.list.jessie;
sudo sudo sh -c 'sed s/jessie/stretch/ /etc/apt/sources.list.jessie > /etc/apt/sources.list';
sudo apt-get update;
sudo apt-get install gcc-6 g++-6;
sudo mv /etc/apt/sources.list.jessie /etc/apt/sources.list;
sudo apt-get update;

# Samba config

# Build & install libraries
cd ~/hackerboat/embedded_software/unified/submodules;
git submodule init;
git submodule update;
cd ~/hackerboat/embedded_software/unified/submodules/GeographicLib;
mkdir BUILD;
cd BUILD;
cmake -D CMAKE_CXX_COMPILER=/usr/bin/g++-6 -D CMAKE_C_COMPILER=/usr/bin/gcc-6; .
make;
sudo make install;

cd ~/hackerboat/embedded_software/unified/submodules/paho.mqtt.c;
mkdir BUILD;
cd BUILD;
cmake -D CMAKE_CXX_COMPILER=/usr/bin/g++-6 -D CMAKE_C_COMPILER=/usr/bin/gcc-6; .
make;
sudo make install;

cd ~/hackerboat/embedded_software/unified/submodules/jansson;
mkdir BUILD;
cd BUILD;
cmake -D CMAKE_CXX_COMPILER=/usr/bin/g++-6 -D CMAKE_C_COMPILER=/usr/bin/gcc-6; .
make;
sudo make install;

# download and install magnetic model
sudo geographiclib-get-magnetic all

# general setup
sudo ldconfig;
sudo cp ~/hackerboat/embedded_software/unified/setup/99-gpio.rules /etc/udev/rules.d;
