IMPACT
-----

Multiphysics application coupling toolkit

## Getting Started ##
To acquire IMPACT, you can clone it with the following command:
```
$ git clone https://github.com/IllinoisRocstar/IMPACT.git
```
## Build Instructions ##
### Build Dependencies ###
Make sure to `apt install` following before you start

* build-essential
* cmake
* mpich
* libcgns-dev
* libhdf4-dev
* liblapack-dev
* libblas-dev
* libjpeg-dev

all of these can be obtain using linux $apt-get install$ command.

## Build IMPACT ##
For the following steps we assume `$IMPACT_PROJECT_PATH` is the path to IMPACT, `$IMPACT_INSTALL_PATH` is 
the desired installation location.
Start the build process by executing:

```
$ cd $IMPACT_PROJECT_PATH
$ mkdir build && cd build
$ cmake -DCMAKE_INSTALL_PREFIX=$IMPACT_INSTALL_PATH .. 
$ make -j
$ make install
```

Executing the commands above will build all libraries and executables.
