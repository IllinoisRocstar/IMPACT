cmake_minimum_required(VERSION 2.8)
project(IMPACT)

# TODO:
# All tests are ruled out for the moment as they
# are all dependent on IRAD which is removed from
# the project

# Setup switches
set (BUILD_STATIC FALSE CACHE BOOL "Build static libraries")
set (ENABLE_MPI TRUE CACHE BOOL "Build with MPI Support")
set (ENABLE_CGNS FALSE CACHE BOOL "Build with CGNS Support")

# Compiled objects output paths
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single directory for all executables.")
set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib CACHE PATH "Single directory for all libraries and archives.")
mark_as_advanced (LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH)

# Set up RPATH
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Dependencies
IF(NOT ENABLE_MPI)
  add_definitions( -DDUMMY_MPI )
ELSE()
  FIND_PACKAGE(MPI REQUIRED)
  add_definitions( -DMPICH_IGNORE_CXX_SEEK )
  INCLUDE_DIRECTORIES(${MPI_INCLUDE_PATH})
ENDIF()

IF(ENABLE_CGNS)
  find_package(HDF5)
  find_library(CGNS_LIB cgns)
  find_path(CGNS_INC cgnslib.h)
  add_definitions ( -DUSE_CGNS )
  INCLUDE_DIRECTORIES(${CGNS_INC})
ENDIF()

# HDF4 libs
find_library(HDF4_LIB NAMES hdf4 libdf df PATHS /usr/lib /usr/lib64 HINTS df libdf hdf)
find_library(MFHDF_LIB NAMES mfhdf4 mfhdf PATHS /usr/lib /usr/lib64 HINTS mfhdf mfhdf4)

# Building modules
IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/COM/")
  add_subdirectory(COM)
  SET(COM_INCPATH ${CMAKE_SOURCE_DIR}/COM/include)
ENDIF()
include_directories(${COM_INCPATH})
IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SimIO/")
  add_subdirectory(SimIO)
ENDIF()
IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Simpal/")
  add_subdirectory(Simpal)
ENDIF()
IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SurfUtil/")
  add_subdirectory(SurfUtil)
ENDIF()
IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SurfMap/")
  add_subdirectory(SurfMap)
ENDIF()
IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SurfX/")
  add_subdirectory(SurfX)
ENDIF()
IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SIM/")
  add_subdirectory(SIM)
ENDIF()
IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Rocface/")
  ADD_SUBDIRECTORY(Rocface)
ENDIF()
IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Rocsurf/")
  ADD_SUBDIRECTORY(Rocsurf)
ENDIF()
IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Rocmap/")
  ADD_SUBDIRECTORY(Rocmap)
ENDIF()

# Project-specific constructs
include_directories(include ./COM/include ./SIM/include)

# Documentation
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" OFF)
IF(BUILD_DOCUMENTATION)
  FIND_PACKAGE(Doxygen)
  IF(NOT DOXYGEN_FOUND)
    MESSAGE(FATAL_ERROR "Doxygen is needed to build the documentation.")
  ENDIF()
  SET( doxyfile_in          ${CMAKE_CURRENT_SOURCE_DIR}/doc/IMPACT.cfg.in )
  SET( doxyfile             ${CMAKE_CURRENT_BINARY_DIR}/IMPACT.cfg        )
  SET( documentation        ${CMAKE_CURRENT_BINARY_DIR}/docs                 )
  CONFIGURE_FILE( ${doxyfile_in} ${doxyfile} @ONLY )
  ADD_CUSTOM_COMMAND( OUTPUT ${documentation}
                      COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
                      MAIN_DEPENDENCY ${doxyfile} ${doxyfile_in}
                      DEPENDS ${doxyfile} ${doxy_extra_files}
                      COMMENT "Generating documentation")
  ADD_CUSTOM_TARGET( docs ALL DEPENDS ${documentation} )
  INSTALL( DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs DESTINATION share/IMPACT )
ENDIF()
