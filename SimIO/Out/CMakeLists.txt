cmake_minimum_required(VERSION 3.1)

# Options
option(ENABLE_CGNS "Build with CGNS support." OFF)
option(USE_PTHREADS "Build threaded library." OFF)

# Find CGNS package
#if(ENABLE_CGNS)
#  find_package(HDF5 REQUIRED)
#  find_library(CGNS_LIB cgns)
#  if(NOT CGNS_LIB)
#    message(FATAL_ERROR "CGNS library not found.")
#  endif()
#  find_path(CGNS_INC cgnslib.h)
#  add_definitions(-DUSE_CGNS)
#endif()

if(USE_PTHREADS)
  find_library(PTHREAD_LIB pthread)
  if(NOT PTHREAD_LIB)
    message(FATAL_ERROR "pthread library not found.")
  endif()
  add_definitions(-DUSE_PTHREADS)
endif()

#find_path(HDF4_INC hdf.h HINT /usr/include/hdf)

# rpath settings
#set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(SimOUT
    src/Rocout.C
    src/Rocout_hdf4.C
    src/write_parameter_file.C
)

if(ENABLE_CGNS)
  target_sources(SimOUT PRIVATE
      src/Rocout_cgns.C
  )
  target_link_libraries(SimOUT ${CGNS_LIBRARY} ${HDF5_LIBRARIES})
  target_include_directories(SimOUT
      PUBLIC
          ${CGNS_INCLUDE_PATH}
          ${HDF5_INCLUDE_DIRS})
endif()

target_include_directories(SimOUT
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${HDF4_INC}>
        $<INSTALL_INTERFACE:include/impact>
)

target_link_libraries(SimOUT SITCOM RHDF4)

# Utilities
if(USE_CGNS)
  add_executable(ghostbuster util/ghostbuster.C)
  target_link_libraries(ghostbuster SimOUT)
endif()

# install the headers and export the targets
install(DIRECTORY include/ 
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/impact)
install(TARGETS SimOUT
    EXPORT IMPACT
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
)
if(USE_CGNS)
  install(TARGETS ghostbuster
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib
      INCLUDES DESTINATION include
  )
endif()
