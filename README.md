IMPACT
-----

Multiphysics application coupling toolkit

## Getting Started ##
To acquire IMPACT, you can clone it with the following command:
```
$ git clone git@git.illinois.rocstar:IR/IMPACT-NO-IRAD.git
```
## Build Instructions ##
### Dependencies ###
Make sure to `apt install` following before you start

* build-essential
* cmake
* mpich
* libcgns-dev
* libhdf4-dev
* liblapack-dev
* libblas-dev
* libjpeg-dev

all of these can be obtained using linux $apt-get install$ command.

### Build IMPACT ###
*NOTE* Currently IMPACT is only tested with MPICH compilers. If you have both OpenMPI and MPICH installed make sure `mpicxx`, `mpicc`, and `mpif90` point to MPICH. In the following we have assumed both MPI libraries are installed.

In the following, we assume `$IMPACT_PROJECT_PATH` is the path to the IMPACT, and `$IMPACT_INSTALL_PATH` is the desired installation location.
Start the build process by executing:

```
$ cd $IMPACT_PROJECT_PATH
$ mkdir build && cd build
$ cmake -DCMAKE_INSTALL_PREFIX=$IMPACT_INSTALL_PATH -DMPI_C_COMPILER=/usr/bin/mpicc.mpich -DMPI_CXX_COMPILER=/usr/bin/mpicxx.mpich -DMPI_Fortran_COMPILER=/usr/bin/mpif90.mpich -DCMAKE_C_COMPILER=mpicc.mpich -DCMAKE_CXX_COMPILER=mpicxx.mpich -DCMAKE_Fortran_COMPILER=mpif90.mpich .. 
$ make -j6
$ make install
```

Executing the commands above will build all libraries and executables.

### Testing IMPACT ###
*NOTE* The testing is currently heavily dependent on IR's IRAD project. Modern testing is currently under development. In case needed, IRAD testing framework can be revived temporarily. Please contact developers for further instructions.

To perform testing, execute following in the build directory:
```
$ make test
```
The output of tests are captured in `$ROCSTAR_PROJECT_PATH/testing`. The testing framework also keeps a log of the test outputs in `$ROCSTAR_PROJECT_PATH/Testing` directory. If tests fail seek output log in this directory for more details.
