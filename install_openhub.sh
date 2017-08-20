#!/bin/bash

#install influxdb, install grafana, create influxdb db,user
sudo echo 'deb http://repos.azulsystems.com/debian stable main' > /etc/apt/sources.list.d/zulu.list
sudo apt-get update
apt-get install zulu-8 wget apt-transport-https -y

wget -qO - 'https://bintray.com/user/downloadSubjectPublicKey?username=openhab' | sudo apt-key add -

echo 'deb https://dl.bintray.com/openhab/apt-repo2 stable main' | sudo tee /etc/apt/sources.list.d/openhab2.list

sudo apt-get update

sudo apt-get install openhab2 -y

sudo systemctl start openhab2.service
sudo systemctl status openhab2.service

sudo systemctl daemon-reload
sudo systemctl enable openhab2.service

#wait 20 min

#http://openhab-device:8080 

sudo wget http://repo.mosquitto.org/debian/mosquitto-repo.gpg.key
sudo apt-key add mosquitto-repo.gpg.key

cd /etc/apt/sources.list.d/

sudo wget http://repo.mosquitto.org/debian/mosquitto-jessie.list

curl -O http://repo.mosquitto.org/debian/mosquitto-repo.gpg.key 
sudo apt-key add mosquitto-repo.gpg.key 
rm mosquitto-repo.gpg.key 
cd /etc/apt/sources.list.d/ 
sudo curl -O 
sudo apt-get update
sudo apt-get install mosquitto mosquitto-clients python-mosquitto

sudo apt-get install openhab-addon-action-astro openhab-addon-action-mail openhab-addon-action-mqtt openhab-addon-action-telegram openhab-addon-action-weather openhab-addon-action-xbmc openhab-addon-binding-astro openhab-addon-binding-exec openhab-addon-binding-http openhab-addon-binding-mqtt openhab-addon-binding-weather openhab-addon-binding-xbmc openhab-addon-io-myopenhab openhab-addon-persistence-db4o openhab-addon-persistence-exec openhab-addon-persistence-gcal openhab-addon-persistence-logging openhab-addon-persistence-mqtt openhab-addon-persistence-mysql openhab-addon-persistence-rrd4j 
