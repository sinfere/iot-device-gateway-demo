# iot-device-gateway-demo

### Install prerequisites
----------------------------

Make and Install `https://github.com/json-c/json-c.git`

#### On CentOS

`yum install -y automake libtool libev libev-libevent-devel`

#### On Ubuntu

`sudo apt install make automake libtool libev-dev`

### Build instructions:
----------------------------
```sh
$ git submodule update --init --recursive
$ ./build.sh
$ ./run.sh
```
