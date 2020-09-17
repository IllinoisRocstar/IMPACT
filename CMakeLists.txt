cmake_minimum_required(VERSION 3.1)

# set project metadata #########################################################

set(IMPACT_MAJOR_VERSION 2)
set(IMPACT_MINOR_VERSION 1)
set(IMPACT_PATCH_VERSION 3)
set(IMPACT_EXTRA_VERSION "" CACHE STRING "IMPACT extra version string")
set(IMPACT_VERSION "${IMPACT_MAJOR_VERSION}.${IMPACT_MINOR_VERSION}")
set(IMPACT_VERSION
    "${IMPACT_VERSION}.${IMPACT_PATCH_VERSION}${IMPACT_EXTRA_VERSION}")
set(IMPACT_SHORT_LICENSE "University of Illinois/NCSA Open Source License")
set(IMPACT_HOMEPAGE_URL "http://git.illinois.rocstar/IR/IMPACT-NO-IRAD")
set(IMPACT_AUTHOR
    "Masoud Safdari (lead)"
    "Alessandro Gondolo"
    "Michael Pope"
    "Taiyo Wilson")
set(IMPACT_AUTHOR_EMAIL "msafdari@illinoisrocstar.com")
set(IMPACT_PYTHON_CLASSIFIERS
    "Development Status :: 2 - Pre-Alpha"
    "License :: OSI Approved :: University of Illinois/NCSA Open Source License"
    "Natural Language :: English"
    "Operating System :: POSIX"
    "Programming Language :: C"
    "Programming Language :: C++"
    "Programming Language :: Fortran"
    "Topic :: Scientific/Engineering")
set(IMPACT_DESCRIPTION "Multiphysics application coupling toolkit")
set(PROJECT_NAME "IMPACT version ${IMPACT_VERSION}")
project(${PROJECT_NAME} CXX C Fortran)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set project defaults #########################################################

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build.")
endif()
# Set the possible values of build type for cmake-gui
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")

# creating code compilation database
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Installation Directories
include(GNUInstallDirs)

# Setting configure options ####################################################

# Options
option(BUILD_SHARED_LIBS "Build shared libraries." ON)
option(ENABLE_TESTS "Build with tests." OFF)

set(IO_FORMAT_DEFAULT "CGNS")
set(IO_FORMAT_OPTIONS "CGNS" "HDF4")
set(IO_FORMAT_DOC "Set I/O module format")

set(IO_FORMAT ${IO_FORMAT_DEFAULT} CACHE STRING ${IO_FORMAT_DOC})
set_property(CACHE IO_FORMAT PROPERTY STRINGS ${IO_FORMAT_OPTIONS})

#option(ENABLE_MPI "Build with MPI support." ON) # Set to always ON
set(ENABLE_MPI ON)

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
  # -Wshadow gives false positives for gcc versions <= 4.8
  if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.9)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow")
  endif (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.9)
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfloat-conversion")
endif()

#mark_as_advanced(LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH)

# set up RPATH
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Finding libraries ############################################################

if(ENABLE_MPI)
  find_package(MPI REQUIRED)
  include_directories(${MPI_INCLUDE_PATH})
  if(MPI_LIBRARY)
    set(MPI_LIBRARIES ${MPI_LIBRARY})
  endif(MPI_LIBRARY)
  if(MPI_EXTRA_LIBRARY)
    set(MPI_LIBRARIES ${MPI_LIBRARIES} "${MPI_EXTRA_LIBRARY}")
  endif(MPI_EXTRA_LIBRARY)
  # Needed for mpich 2
  add_definitions(-DMPICH_IGNORE_CXX_SEEK)
else()
  add_definitions(-DDUMMY_MPI)
endif()

if("${IO_FORMAT}" STREQUAL "CGNS")
  # CGNS requires HDF5
  find_package(HDF5 REQUIRED COMPONENTS CXX)
  list(GET ${HDF5_LIBRARIES} 0 HDF5_LIBRARY)
  if(NOT HDF5_FOUND)
    message(FATAL_ERROR "HDF5 library not found.")
  endif()
  get_filename_component(HDF5_LIB_PATH ${HDF5_LIBRARY} PATH)

  # Find CGNS lib
  find_library(CGNS_LIBRARY cgns)
  if(NOT CGNS_LIBRARY)
    message(FATAL_ERROR "CGNS library not found.")
  endif()
  find_file(CGNS_HDR cgnslib.h)
  get_filename_component(CGNS_INCLUDE_PATH ${CGNS_HDR} PATH)
elseif("${IO_FORMAT}" STREQUAL "HDF4")
  # Find HDF4 and MFHDF libs
  find_library(HDF4_LIB NAMES hdf4 libdf df dfalt
      PATHS /usr/lib /usr/lib64
      HINTS df libdf hdf)
  if(NOT HDF4_LIB)
    message(FATAL_ERROR "HDF4 library not found.")
  endif()
  find_path(HDF4_INC hdf.h HINT /usr/include/hdf)
  find_path(MFHDF4_INC mfhdf.h HINT /usr/include/hdf)
  find_library(MFHDF_LIB NAMES mfhdf4 mfhdf mfhdfalt
      PATHS /usr/lib /usr/lib64
      HINTS mfhdf mfhdf4)
  if(NOT MFHDF_LIB)
    message(FATAL_ERROR "MFHDF library not found.")
  endif()
else()
  message(FATAL_ERROR "IO format ${IO_FORMAT} is not supported.")
endif()
message(STATUS "IO format set to ${IO_FORMAT}.")

# Add project submodules #######################################################

# Building modules
add_subdirectory(COM)
add_subdirectory(SimIO)
add_subdirectory(Simpal)
add_subdirectory(SurfUtil)
add_subdirectory(SurfMap)
add_subdirectory(SurfX)
add_subdirectory(SIM)

# Perform Testing ##############################################################

if(ENABLE_TESTS)
  enable_testing()
  add_subdirectory(SolverUtils)
  add_subdirectory(testing)
endif(ENABLE_TESTS)

# Installation #################################################################

# Export IMPACT as a CMake package
include(CMakePackageConfigHelpers)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/IMPACTVersion.cmake
    VERSION ${IMPACT_VERSION}
    COMPATIBILITY SameMajorVersion)
install(FILES IMPACTConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/IMPACTVersion.cmake
        DESTINATION lib/cmake/IMPACT)
install(EXPORT IMPACT
        NAMESPACE IMPACT::
        DESTINATION lib/cmake/IMPACT)

# Documentation ################################################################

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
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs
          DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/IMPACT)
endif()

# Packaging ####################################################################

set(CPACK_PACKAGE_NAME "IMPACT")
set(CPACK_PACKAGE_VENDOR "${IMPACT_AUTHOR}")
set(CPACK_PACKAGE_VERSION "${IMPACT_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR "${IMPACT_MAJOR_VERSION}")
set(CPACK_PACKAGE_VERSION_MINOR "${IMPACT_MINOR_VERSION}")
set(CPACK_PACKAGE_VERSION_PATCH "${IMPACT_PATCH_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION "${IMPACT_DESCRIPTION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${IMPACT_DESCRIPTION}")
set(CPACK_PACKAGE_HOMEPAGE_URL "")
set(CPACK_PACKAGE_CONTACT "${IMPACT_AUTHOR_EMAIL}")
#set(CPACK_PACKAGE_INSTALL_DIRECTORY "") # Use default location.
#set(CPACK_PACKAGE_ICON "") # image used by GUI installers
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

if (APPLE)
    #TODO
elseif (UNIX)
    file(READ "/etc/os-release" OS_INFO)
    string(REGEX MATCH "Ubuntu|CentOS" DIST ${OS_INFO})
    if (DIST STREQUAL "Ubuntu")
      set(Debian TRUE)
    elseif (DIST STREQUAL "CentOS")
      set(CentOS TRUE)
    endif ()
endif ()

if(WIN32)
  set(CPACK_GENERATOR NSIS)
elseif (Debian)
  if(CMAKE_VERSION VERSION_LESS 3.10)
    message(WARNING "CMake 3.10 or above is required to build a DEB package.")
  else()
    set(CPACK_GENERATOR DEB)

    # set Debian Packaging settings
    #set(CPACK_DEBIAN_PACKAGE_NAME "")
    #set(CPACK_DEBIAN_PACKAGE_VERSION "")
    #set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "")
    #set(CPACK_DEBIAN_PACKAGE_MAINTAINER "")
    #set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "")
    set(CPACK_DEBIAN_PACKAGE_SECTION "science")
    #set(CPACK_DEBIAN_COMPRESSION_TYPE "gzip")
    #set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "${CPACK_PACKAGE_HOMEPAGE_URL}")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_PACKAGE_DEBUG ON)  # For testing only
    #set(CPACK_DEBIAN_PACKAGE_PREDEPENDS "")
    #set(CPACK_DEBIAN_PACKAGE_ENHANCES "")
    #set(CPACK_DEBIAN_PACKAGE_BREAKS "")
    #set(CPACK_DEBIAN_PACKAGE_CONFLICTS "")
    #set(CPACK_DEBIAN_PACKAGE_PROVIDES "")
    #set(CPACK_DEBIAN_PACKAGE_REPLACES "")
    #set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "")
    #set(CPACK_DEBIAN_PACKAGE_SUGGESTS "")
    #set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "")

    set(CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION TRUE)  # CMake 3.4
    #set(CPACK_DEBIAN_PACKAGE_SOURCE "")  # CMake 3.5
    #set(CPACK_DEB_COMPONENT_INSTALL OFF)  # CMake 3.6
    set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")  # CMake 3.6
    #set(CPACK_DEBIAN_PACKAGE_RELEASE "1")  # CMake 3.6
    set(CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS ON)  # CMake 3.6
    #set(CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS_POLICY "=")  # CMake 3.6
    #set(CPACK_DEBIAN_ARCHIVE_TYPE "gnutar")  # CMake 3.7
    #set(CPACK_DEBIAN_PACKAGE_EPOCH "")  # Cmake 3.10
  endif()
elseif (CentOS)
  set(CPACK_GENERATOR RPM)

  # set RPM package settings
  # set(CPACK_RPM_COMPONENT_INSTALL "") for multiple packages
  # set(CPACK_RPM_PACKAGE_SUMMARY "")
  # set(CPACK_RPM_PACKAGE_NAME "")
  set(CPACK_RPM_FILE_NAME "RPM-DEFAULT")
  # set(CPACK_RPM_MAIN_COMPONENT "")
  # set(CPACK_RPM_PACKAGE_EPOCH "") for package numbering when the version # doesn't change
  # set(CPACK_RPM_PACKAGE_VERSION "")
  # set(CPACK_RPM_PACKAGE_ARCHITECTURE "")
  # set(CPACK_RPM_PACKAGE_RELEASE "")
  # set(CPACK_RPM_PACKAGE_RELEASE_DIST "")
  # set(CPACK_RPM_PACKAGE_LICENSE "") default sets to "unknown"
  set(CPACK_RPM_PACKAGE_GROUP "science")
  set(CPACK_RPM_PACKAGE_VENDOR "IllinoisRocstar")
  set(CPACK_RPM_PACKAGE_URL "${CPACK_PACKAGE_HOMEPAGE_URL}")
  # set(CPACK_RPM_PACKAGE_DESCRIPTION "")
  # set(CPACK_RPM_COMPRESSION_TYPE "")
  set(CPACK_RPM_PACKAGE_AUTOREQ ON) # Enable automatic shared lib dep detection
  set(CPACK_RPM_PACKAGE_AUTOPROV ON) # Enable automatic listing of shared libs
  set(CPACK_RPM_PACKAGE_AUTOREQPROV ON)
  # set(CPACK_RPM_PACKAGE_REQUIRES "")
  # set(CPACK_RPM_PACKAGE_CONFLICTS "")
  # set(CPACK_RPM_PACKAGE_REQUIRES_PRE "")
  # set(CPACK_RPM_PACKAGE_REQUIRES_POST "")
  # set(CPACK_RPM_PACKAGE_REQUIRES_POSTUN "")
  # set(CPACK_RPM_PACKAGE_REQUIES_PREUN "")
  # set(CPACK_RPM_PACKAGE_SUGGESTS "")
  # set(CPACK_RPM_PACKAGE_PROVIDES "")
  # set(CPACK_RPM_PACKAGE_OBSOLETES "")
  # set(CPACK_RPM_PACKAGE_RELOCATABLE OFF)
  # set(CPACK_RPM_SPEC_INSTALL_POST "")
  # set(CPACK_RPM_SPEC_MORE_DEFINE "")
  set(CPACK_RPM_PACKAGE_DEBUG ON)
  # set(CPACK_RPM_USER_BINARY_SPECFILE "")
  # set(CPACK_RPM_GENERATE_USER_BINARY_SPECFILE_TEMPLATE "")
  # set(CPACK_RPM_PRE_INSTALL_SCRIPT_FILE "")
  # set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "")
  # set(CPACK_RPM_USER_FILELIST "")
  # set(CPACK_RPM_CHANGELOG_FILE "")
  # set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST "")
  # set(CPACK_RPM_DEFAULT_USER "") User & group should always be root
  # set(CPACK_RPM_DEFAULT_GROUP "")
else ()
    message(WARNING "Package generation is currently only supported on CentOS and Ubuntu distributions")
endif ()
# Lists exe and text label used for Start Menu shortcuts
#set(CPACK_PACKAGE_EXECUTABLES "")
# Must have matching Start Menu shortcut to create a Desktop link
#set(CPACK_CREATE_DESKTOP_LINKS "")

# WIN32 NOTE: - Run CPack from a MSVC environment as dumpbin is required
#               for the fixup_bundle search of DLL dependecies.
#             - The system libraries pulled in are release DLLs.
#               Do NOT package in Debug! THAT APPLIES TO DEPENDENCIES TOO!!
include(CPack)
