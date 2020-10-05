cmake_minimum_required(VERSION 3.1)

# Options
option(USE_PTHREADS "Build threaded library." OFF)

if(USE_PTHREADS)
  find_library(PTHREAD_LIB pthread)
  if(NOT PTHREAD_LIB)
    message(FATAL_ERROR "pthread library not found.")
  endif()
endif()

add_library(SimOUT
    src/Rocout.C
    src/write_parameter_file.C
)

set_target_properties(SimOUT PROPERTIES VERSION ${IMPACT_VERSION}
        SOVERSION ${IMPACT_MAJOR_VERSION})

if(USE_PTHREADS)
  target_compile_definitions(SimOUT PRIVATE USE_PTHREADS)
endif()

if("${IO_FORMAT}" STREQUAL "CGNS")
  target_sources(SimOUT PRIVATE
      src/Rocout_cgns.C
  )
  target_link_libraries(SimOUT ${CGNS_LIBRARY} ${HDF5_LIBRARIES})
  target_include_directories(SimOUT
      PUBLIC
          ${CGNS_INCLUDE_PATH}
          ${HDF5_INCLUDE_DIRS})
  target_compile_definitions(SimOUT PRIVATE USE_CGNS)
elseif("${IO_FORMAT}" STREQUAL "HDF4")
  target_sources(SimOUT PRIVATE
      src/Rocout_hdf4.C
  )
  target_link_libraries(SimOUT RHDF4)
  target_include_directories(SimOUT
      PUBLIC
        $<BUILD_INTERFACE:${HDF4_INC}>)
  target_compile_definitions(SimOUT PRIVATE USE_HDF4)
endif()

target_include_directories(SimOUT
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/impact>
)

target_link_libraries(SimOUT SITCOM)

# install the headers and export the targets
install(DIRECTORY include/ 
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/impact)
install(TARGETS SimOUT
    EXPORT IMPACT
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Utilities
if("${IO_FORMAT}" STREQUAL "CGNS")
  add_executable(ghostbuster util/ghostbuster.C)
  target_link_libraries(ghostbuster SimOUT)
  install(TARGETS ghostbuster
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
      )
endif()
