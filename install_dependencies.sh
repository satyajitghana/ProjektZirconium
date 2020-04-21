#!/bin/bash

# install mongo-c driver

wget https://github.com/mongodb/mongo-c-driver/releases/download/1.17.0-beta/mongo-c-driver-1.17.0-beta.tar.gz
# install ssl, important
sudo apt install -y libssl-dev libsasl2-dev
tar xvf mongo-c-driver-1.17.0-beta.tar.gz
cd mongo-c-driver-1.17.0-beta/build && cmake -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF .. && make -j8 && sudo make install

# install mongo-cxx driver

curl -OL https://github.com/mongodb/mongo-cxx-driver/releases/download/r3.5.0/mongo-cxx-driver-r3.5.0.tar.gz
tar xvf mongo-cxx-driver-r3.5.0.tar.gz
cd mongo-cxx-driver-r3.5.0/build && cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local && sudo make EP_mnmlstc_core && make -j8 && sudo make install

# install mongodb
wget -qO - https://www.mongodb.org/static/pgp/server-4.2.asc | sudo apt-key add -
echo "deb [ arch=amd64,arm64 ] https://repo.mongodb.org/apt/ubuntu bionic/mongodb-org/4.2 multiverse" | sudo tee /etc/apt/sources.list.d/mongodb-org-4.2.list
sudo apt update && sudo apt install -y mongodb-org
mkdir /data/db
sudo chmod -R go+w /data/db
# sudo service mongodb restart
# start the service by: mongod --dbpath /data/db
