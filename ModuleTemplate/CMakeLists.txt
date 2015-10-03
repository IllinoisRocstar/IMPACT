cmake_minimum_required (VERSION 2.8)
project(TestModule CXX Fortran C)

load_cache(../)
IF(NOT ENABLE_MPI)
  add_definitions( -DDUMMY_MPI )
ELSE()
  FIND_PACKAGE(MPI REQUIRED)
  add_definitions( -DMPICH_IGNORE_CXX_SEEK )
ENDIF()

find_path(COM_INC roccom.h HINTS ../COM/include)
find_path(SimIN_INC HDF4.h HINTS ../SimIO/In/include)

set (LIB_SRCS src/TestModule.C src/TestFortranModule.f90)
set (UTIL_SRCS src/TestModuleDriver.C src/TestModuleDriver.f90)
set (ALL_SRCS ${LIB_SRCS} ${UTIL_SRCS})

set_source_files_properties(${ALL_SRCS} PROPERTIES COMPILE_FLAGS "-fPIC" )
#set(TEST_SRCS src/MANTest.C)

# rpath settings
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

include_directories(include ${COM_INC} ${SimIN_INC} ${CMAKE_BINARY_DIR}/include)

IF(NOT BUILD_STATIC)
  add_library(TestModule SHARED ${LIB_SRCS})
  add_library(TestFortranModule SHARED ${LIB_SRCS})
ELSE()
  add_library(TestModule STATIC ${LIB_SRCS})
  add_library(TestFortranModule STATIC ${LIB_SRCS})
  add_definitions( -DSTATIC_LINK )
ENDIF()
target_link_libraries(TestModule SITCOM SITCOMF)
target_link_libraries(TestFortranModule SITCOM SITCOMF)

add_executable(TestModuleDriver src/TestModuleDriver.C)
target_link_libraries(TestModuleDriver TestModule TestFortranModule)

add_executable(TestModuleDriverF src/TestModuleDriver.f90)
set_target_properties(TestModuleDriverF PROPERTIES LINKER_LANGUAGE Fortran)
target_link_libraries(TestModuleDriverF TestModule TestFortranModule)

#ADD_TEST(RunAllTests ${EXECUTABLE_OUTPUT_PATH}/testx testresults.txt)
#ADD_TEST(TestObject:ValidHandle ${TEST_RESULTS} TestObject:ValidHandle testresults.txt)
#ADD_TEST(TestObject:Exists ${TEST_RESULTS} TestObject:Exists testresults.txt)

INSTALL(TARGETS TestModule TestFortranModule TestModuleDriver RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(TARGETS TestModule TestFortranModule TestModuleDriverF RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
