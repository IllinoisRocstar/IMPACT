cmake_minimum_required(VERSION 2.8)
project(IMPACT)

set (BUILD_STATIC FALSE CACHE BOOL "Build static libraries")
set (ENABLE_MPI TRUE CACHE BOOL "Build with MPI Support")

# Compiled objects output paths
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single directory for all executables.")
set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib CACHE PATH "Single directory for all libraries and archives.")
mark_as_advanced (LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH)
# Set up RPATH
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

IF(NOT ENABLE_MPI)
  add_definitions( -DDUMMY_MPI )
ELSE()
  FIND_PACKAGE(MPI REQUIRED)
  add_definitions( -DMPICH_IGNORE_CXX_SEEK )
  INCLUDE_DIRECTORIES(${MPI_INCLUDE_PATH})
ENDIF()

# 
# ========= USE IN EXISTING PROJECT ============
#
# In order to use this IllinoisRocstar Project Template
# in an existing project COPY this file from this point
# on and paste it into your project's CMakeLists.txt 
# file *before* your existing INSTALL directives.
#
INCLUDE(CTest)
IF(BUILD_TESTING)
  SET(BUILDNAME "${BUILDNAME}" CACHE STRING "Name of build on the dashboard")
  MARK_AS_ADVANCED(BUILDNAME)
ENDIF(BUILD_TESTING)

IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/IRAD/")
  add_subdirectory(IRAD)
  SET(IRAD_INCPATH ${CMAKE_CURRENT_SOURCE_DIR}/IRAD/include)
  SET(RUNTEST ${EXECUTABLE_OUTPUT_PATH}/runtest)
  SET(TESTRESULT ${EXECUTABLE_OUTPUT_PATH}/testresults)
  SET(IRAD_LIB IRAD)
ELSE()
  FIND_LIBRARY(IRAD_LIB IRAD)
  FIND_FILE(IRAD_HDR COMM.H)
  FIND_PROGRAM(RUNTEST runtest)
  FIND_PROGRAM(TESTRESULT testresults)
  GET_FILENAME_COMPONENT(IRAD_LIBPATH ${IRAD_LIB} PATH)
  GET_FILENAME_COMPONENT(IRAD_INCPATH ${IRAD_HDR} PATH)
  GET_FILENAME_COMPONENT(IRAD_EXEPATH ${RUNTEST} PATH)
#  link_directories(${IRAD_LIBPATH})
ENDIF()
GET_DIRECTORY_PROPERTY(hasParent PARENT_DIRECTORY)

IF(hasParent)
  SET(IRAD_INCPATH ${IRAD_INCPATH} PARENT_SCOPE)
  SET(IRAD_LIB ${IRAD_LIB} PARENT_SCOPE)
  SET(IRAD_LIBPATH ${IRAD_LIBPATH} PARENT_SCOPE)
  SET(RUNTEST ${RUNTEST} PARENT_SCOPE)
  SET(TESTRESULT ${TESTRESULT} PARENT_SCOPE)
  SET(IRAD_EXEPATH ${IRAD_EXEPATH} PARENT_SCOPE)
ENDIF()

include_directories(${IRAD_INCPATH})
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
IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ModuleTemplate/")
  add_subdirectory(ModuleTemplate)
ENDIF()
IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/TestModule/")
  add_subdirectory(TestModule)
ENDIF()
IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SolverUtils/")
  ADD_SUBDIRECTORY(SolverUtils)
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
IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/testing/")
  add_subdirectory(testing)
ENDIF()

# Project-specific constructs
include_directories(include ./COM/include ./SIM/include)


# Add target to build the documentation
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

# ========= USE IN EXISTING PROJECT ==============
