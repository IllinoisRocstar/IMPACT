IMPACT
-----

Multiphysics application coupling toolkit

## Version ##
Version 2.1.1

IMPACT follows semantic versioning. The version will be major.minor.patch.
We will:
* Increase the version for bug fixes, security fixes, and code
documentation. Backwards compatible; no breaking changes.
* Increase the minor version for new features and additions to the library’s
interface. Backwards compatible; no breaking changes.
* Increase the major version for breaking changes to the library’s interface or
breaking changes to behavior.

## Getting Started ##
To acquire IMPACT, you can clone it with the following command:
```
$ git clone https://github.com/IllinoisRocstar/IMPACT.git
```
## Build Instructions ##
### Dependencies ###
Make sure to `apt install` following before you start

* build-essential
* cmake
* liblapack-dev
* libblas-dev
* libjpeg-dev
* EITHER mpich OR openmpi
* EITHER libcgns-dev OR libhdf4-dev

all of these can be obtained using linux `apt-get install` command.

### Build IMPACT ###
**NOTE** Currently IMPACT is only tested with MPI compilers. Serial build is not recommended.

In the following, we assume `$IMPACT_PROJECT_PATH` is the path to the IMPACT directory, and `$IMPACT_INSTALL_PATH` is the desired installation location.
Start the build process by executing:

```
$ cd $IMPACT_PROJECT_PATH
$ mkdir build && cd build
$ cmake -DCMAKE_INSTALL_PREFIX=$IMPACT_INSTALL_PATH .. 
$ make -j$(nproc)
$ make install
```

Executing the commands above will build all libraries and executables.

**NOTE** The CMake variables can also be set by using `ccmake .` from the build directory.

### Testing IMPACT ###

To perform testing, be sure to turn on the `ENABLE_TESTS` CMake variable. This can be done by adding `-DENABLE_TESTS=ON` to the cmake command listed above, or by using the ccmake GUI. After enabling tests be sure to recompile and execute the following in the build directory:
```
$ make test
```
Currently most of the tests rely on CGNS input files, so only a fraction of the tests will run if `IO_FORMAT=HDF4`. The output of tests are captured in `$IMPACT_PROJECT_PATH/build/testing/data`. The testing framework also keeps a log of the test outputs in `$IMPACT_PROJECT_PATH/build/Testing/Temporary` directory. If tests fail seek output log in this directory for more details.
