##!/bin/bash

sudo apt update
sudo apt install make
sudo apt install gnuplot gnuplot-x11 gnuplot-doc
echo "\n#############################################"
echo "# Installation Fertig. Führe Makefiles aus: #"
echo "#############################################"
make
