cmake_minimum_required(VERSION 3.1)

# set project metadata #########################################################

set(IMPACT_MAJOR_VERSION 1)
set(IMPACT_MINOR_VERSION 0)
set(IMPACT_PATCH_VERSION 0)
set(IMPACT_EXTRA_VERSION "" CACHE STRING "IMPACT extra version string")
set(IMPACT_VERSION "${IMPACT_MAJOR_VERSION}.${IMPACT_MINOR_VERSION}")
set(IMPACT_VERSION
    "${IMPACT_VERSION}.${IMPACT_PATCH_VERSION}${IMPACT_EXTRA_VERSION}")
set(IMPACT_SHORT_LICENSE "University of Illinois/NCSA Open Source License")
set(IMPACT_HOMEPAGE_URL "http://git.illinois.rocstar/IR/IMPACT-NO-IRAD")
set(IMPACT_AUTHOR
    "Masoud Safdari (lead)"
    "Alessandro Gondolo"
    "Taiyo Wilson"
    "Michael Pope")
set(IMPACT_AUTHOR_EMAIL "msafdari@illinoisrocstar.com")
# TODO: Update the Python Classifier information
set(IMPACT_PYTHON_CLASSIFIERS
    "Development Status :: 2 - Pre-Alpha"
    "License :: OSI Approved :: "
    "Natural Language :: English"
    "Operating System :: Microsoft :: Windows :: Windows 10"
    "Operating System :: POSIX"
    "Programming Language :: C"
    "Programming Language :: C++"
    "Programming Language :: Fortran"
    "Topic :: Scientific/Engineering")
set(IMPACT_DESCRIPTION "Multiphysics application coupling toolkit")
set(PROJECT_NAME "IMPACT version ${IMPACT_VERSION}")
project(${PROJECT_NAME} CXX C Fortran)

#should this be turned off? Modern CMake messiah says to allow CMake to figure
#it out on its own
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set project defaults #########################################################
set(default_build_type "Release")
# creating code compilation database
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# do not install gtest when installing this project
set(INSTALL_GTEST OFF)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries." ON)
option(ENABLE_MPI "Build with MPI support." ON)
option(ENABLE_CGNS "Build with CGNS support." ON)
option(ENABLE_TESTS "Build with tests." OFF)

# Setting compile and link options #############################################

# Compiled objects output paths
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Turn on default warnings for GNU compilers.
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-sign-compare")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wno-unused-parameter")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual -Wnon-virtual-dtor")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wstrict-null-sentinel -Wredundant-decls")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow")
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfloat-conversion")
endif()

#mark_as_advanced(LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH)

# set up RPATH
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Finding libraries ############################################################
if(NOT ENABLE_MPI)
  add_definitions( -DDUMMY_MPI )
else()
  find_package(MPI REQUIRED)
  include_directories(${MPI_INCLUDE_PATH})
  if(MPI_LIBRARY)
    set(MPI_LIBRARIES ${MPI_LIBRARY})
  endif(MPI_LIBRARY)
  if(MPI_EXTRA_LIBRARY)
    set(MPI_LIBRARIES ${MPI_LIBRARIES} "${MPI_EXTRA_LIBRARY}")
  endif(MPI_EXTRA_LIBRARY)
  # Needed for mpich 2
  add_definitions( -DMPICH_IGNORE_CXX_SEEK )
endif()

# Find HDF4 libs
find_library(HDF4_LIB NAMES hdf4 libdf df PATHS /usr/lib /usr/lib64 HINTS df libdf hdf)
find_library(MFHDF_LIB NAMES mfhdf4 mfhdf PATHS /usr/lib /usr/lib64 HINTS mfhdf mfhdf4)

# Find CGNS lib
if(ENABLE_CGNS)
  # Find HDF5 libs
  find_package(HDF5 REQUIRED COMPONENTS CXX)
  list(GET ${HDF5_LIBRARIES} 0 HDF5_LIBRARY)
  if(NOT HDF5_FOUND)
    message(FATAL_ERROR "HDF5 library not found.")
  endif()
  get_filename_component(HDF5_LIB_PATH ${HDF5_LIBRARY} PATH)

  find_library(CGNS_LIBRARY cgns)
  if(NOT CGNS_LIBRARY)
    message(FATAL_ERROR "CGNS library not found.")
  endif()
  find_file(CGNS_HDR cgnslib.h)
  get_filename_component(CGNS_INCLUDE_PATH ${CGNS_HDR} PATH)
  add_definitions(-DUSE_CGNS)
endif()

# Building modules
add_subdirectory(COM)
add_subdirectory(SimIO)
add_subdirectory(Simpal)
add_subdirectory(SurfUtil)
add_subdirectory(SurfMap)
add_subdirectory(SurfX)
add_subdirectory(SIM)
add_subdirectory(SolverUtils)
if(ENABLE_TESTS)
  enable_testing()
  add_subdirectory(testing)
endif(ENABLE_TESTS)

# Export IMPACT as a CMake package
install(FILES IMPACTConfig.cmake
        DESTINATION lib/cmake/IMPACT)
install(EXPORT IMPACT
        NAMESPACE IMPACT::
        DESTINATION lib/cmake/IMPACT)

# Documentation
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" OFF)
if(BUILD_DOCUMENTATION)
  find_package(Doxygen)
  if(NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Doxygen is needed to build the documentation.")
  endif()
  set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/doc/IMPACT.cfg.in)
  set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/IMPACT.cfg)
  set(documentation ${CMAKE_CURRENT_BINARY_DIR}/docs)
  configure_file(${doxyfile_in} ${doxyfile} @ONLY)
  add_custom_command(OUTPUT ${documentation}
      COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
      MAIN_DEPENDENCY ${doxyfile} ${doxyfile_in}
      DEPENDS ${doxyfile} ${doxy_extra_files}
      COMMENT "Generating documentation")
  add_custom_target(docs ALL DEPENDS ${documentation})
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs DESTINATION share/IMPACT)
endif()
