`Gateway demo`
==============

Install prerequisites
-----------------------
Make and Install `https://github.com/json-c/json-c.git`

On CentOS
-----------------------
`yum install -y automake libtool libev libev-libevent-devel`

### Build instructions:

```sh
$ ./configure  # --enable-threading
$ make
$ make install
```