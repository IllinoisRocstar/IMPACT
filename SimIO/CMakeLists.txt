cmake_minimum_required (VERSION 2.8)
project(SimIO)
set (BUILD_STATIC FALSE CACHE BOOL "Build static libraries")
set (ENABLE_MPI TRUE CACHE BOOL "Build with MPI Support")

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single directory for all executables.")
set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib CACHE PATH "Single directory for all libraries and archives.")
mark_as_advanced (LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH)
IF(NOT ENABLE_MPI)
  add_definitions( -DDUMMY_MPI )
ELSE()
  FIND_PACKAGE(MPI REQUIRED)
  add_definitions( -DMPICH_IGNORE_CXX_SEEK )
  INCLUDE_DIRECTORIES(${MPI_INCLUDE_PATH})
ENDIF()


add_subdirectory(In)
add_subdirectory(Out)

include_directories(../COM/include ./In/include ./Out/include ${CMAKE_BINARY_DIR}/include)

#INSTALL(TARGETS rocstar RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
