# BankingApp - A high performance server and client using gRPC and MongoDB

## Tech Stack

- gRPC c++
- MongoDB 4.2
- mongocxx driver
- bsoncxx
- c++ 17

## Toolchains Required

1. Install Bazel

```bash
sudo apt install curl
curl https://bazel.build/bazel-release.pub.gpg | sudo apt-key add -
echo "deb [arch=amd64] https://storage.googleapis.com/bazel-apt stable jdk1.8" | sudo tee /etc/apt/sources.list.d/bazel.list
sudo apt update && sudo apt install bazel
```

2. Install CMake

```bash
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | sudo apt-key add -
sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main'
sudo apt update
sudo apt install cmake -y
```

## Build Instructions

1. Install the dependencies

INFO: installs mongo-c-driver, mongo-cxx-driver and mongodb

```bash
sudo sh install_dependencies.sh
```

2. Test your installation

```bash
$ g++ --std=c++11 test.cpp $(pkg-config --cflags --libs libmongocxx) -Wl,-rpath,/usr/local/lib
$ ./a.out
```

3. Build 🚀

```bash
sudo sh build.sh
```

## Running

1. create a replica set in mongo

```shell
$ mongod --dbpath /data/db --replSet "rs0"
```

2. open a mongo shell and initiate the replica set
```
rs.initiate()
```

3. start the server
```shell
$ bazel run //server:zirconium_server --       
```


4. start the client

```shell
$ bazel run //:zirconium_client -- --username=meow --pin=123456 --deposit=10
```
