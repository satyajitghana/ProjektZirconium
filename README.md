<div align="center">
<img src="logo.png" >
</div>

<h1 align="center">Zirconium Bank 🤑</h1>

# A high performance Server and Client using gRPC and MongoDB

## Status ⌚

![zrb-build](https://github.com/satyajitghana/ProjektZirconium/workflows/zrb-build/badge.svg?branch=master)

![language-count](https://img.shields.io/github/languages/count/satyajitghana/ProjektZirconium)
![top-language](https://img.shields.io/github/languages/top/satyajitghana/ProjektZirconium)
![repo-size](https://img.shields.io/github/repo-size/satyajitghana/ProjektZirconium)
![loc-count](https://sloc.xyz/github/satyajitghana/ProjektZirconium)
![license](https://img.shields.io/github/license/satyajitghana/ProjektZirconium)
![maintainer](https://img.shields.io/badge/maintainer-shadowleaf-blue)

## Tech Stack 🧠

- languages used
    - c++ 17
    - skylark
    - protobuf
- libraries
    - mongo-c-driver 1.17.0 beta
    - mongo-cxx-driver 3.5.0
    - gsoncxx
    - cxxopts
    - loguru
- build tools
    - bazel 3.0.0
    - cmake 3.17.1
- database
    - mongodb 4.2

---

## Toolchains Required 🎈

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

---

## Build Instructions 🔨

1. Install the dependencies 🍰

INFO: installs mongo-c-driver, mongo-cxx-driver and mongodb

```bash
sudo sh install_dependencies.sh
```

2. Test your installation 🧪

```bash
$ g++ --std=c++11 test.cpp $(pkg-config --cflags --libs libmongocxx) -Wl,-rpath,/usr/local/lib
$ ./a.out
```

3. Build 🚀

```bash
sudo sh build.sh
```

---

## Running 🎯

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

## Testing

1. Run Mongod

```shell
$ mongod --dbpath /data/db --replSet "rs0"
```

2. Run Mongo Client

```shell
$ mongo
```

3. Create a replica set
```shell
rs.initiate()
```

4. Open Mongo Compass and connect to our database

Use this URI
```shell
mongodb://localhost:27017/?replicaSet=rs0&readPreference=primary&appname=MongoDB%20Compass&ssl=false
```

5. Load the data from test_data.json

`ADD DATA -> Import File`

6. Start the server

```shell
$ ./bazel-bin/server/zirconium_server --config server/server.config
```

7. Now run test_transactions.sh

Make sure to install `parallel` by `sudo apt install parallel`

```shell
$ seq 10 | parallel -j 10 --workdir $PWD ./test/test_transactions.sh {}
```

----

<h3 align="center">Made with 💘 by shadowleaf</h3>
