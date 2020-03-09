cmake_minimum_required(VERSION 3.1)

# Options
option(USE_PTHREADS "Build threaded library." OFF)

if(USE_PTHREADS)
  find_library(PTHREAD_LIB pthread)
  if(NOT PTHREAD_LIB)
    message(FATAL_ERROR "pthread library not found.")
  endif()
  add_definitions(-DUSE_PTHREADS)
endif()

# Find HDF4 and MFHDF libs
find_library(HDF4_LIB NAMES hdf4 libdf df dfalt)# PATHS /usr/lib /usr/lib64 HINTS df libdf hdf)
if(NOT HDF4_LIB)
  message(FATAL_ERROR "HDF4 library not found.")
endif()
find_path(HDF4_INC hdf.h HINT /usr/include/hdf)
find_library(MFHDF_LIB NAMES mfhdf4 mfhdf mfhdfalt)# PATHS /usr/lib /usr/lib64 HINTS mfhdf mfhdf4)
if(NOT MFHDF_LIB)
  message(FATAL_ERROR "MFHDF library not found.")
endif()

add_library(RHDF4
    src/HDF4.C
)

set_target_properties(RHDF4 PROPERTIES VERSION ${IMPACT_VERSION}
        SOVERSION ${IMPACT_MAJOR_VERSION})

if(USE_PTHREADS)
  target_sources(RHDF4 PRIVATE
      src/Sync.C
  )
  target_link_libraries(RHDF4 SITCOM)
endif()

# Find JPEG and Zlib compression Libraries
find_package(JPEG REQUIRED)
find_package(ZLIB REQUIRED)
target_link_libraries(RHDF4 ${MFHDF_LIB} ${HDF4_LIB} ${JPEG_LIBRARIES} ${ZLIB_LIBRARIES})

MESSAGE(STATUS "JPEG_INCLUDE_DIR = ${JPEG_INCLUDE_DIR}")
MESSAGE(STATUS "ZLIB_INCLUDE_DIR = ${ZLIB_INCLUDE_DIR}")

target_include_directories(RHDF4
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${HDF4_INC}>
        $<BUILD_INTERFACE:${ZLIB_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${JPEG_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/impact>
        $<INSTALL_INTERFACE:${JPEG_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:${ZLIB_INCLUDE_DIR}>
)

# install the headers and export the targets
install(DIRECTORY include/ 
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/impact)
install(TARGETS RHDF4
    EXPORT IMPACT
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

add_subdirectory(In)
add_subdirectory(Out)
