cmake_minimum_required (VERSION 2.8)
project(SIM)

load_cache(../)
IF(NOT ENABLE_MPI)
  add_definitions( -DDUMMY_MPI )
ELSE()
  FIND_PACKAGE(MPI REQUIRED)
  add_definitions( -DMPICH_IGNORE_CXX_SEEK )
  INCLUDE_DIRECTORIES(${MPI_INCLUDE_PATH})
ENDIF()

#INCLUDE(CTest)
find_path(COM_INC com.h HINTS ../COM/include) 
find_path(IO_INC HDF4.h HINTS ../SimIO/In/include) 
set (LIB_SRCS src/Action.C src/Scheduler.C src/Coupling.C src/Agent.C ) 
set (ALL_SRCS ${LIB_SRCS})

set_source_files_properties(${ALL_SRCS} PROPERTIES COMPILE_FLAGS "-fPIC ${MPI_CXX_COMPILE_FLAGS}" )
#set(TEST_SRCS src/MANTest.C)

# rpath settings
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

include_directories(include ${CMAKE_BINARY_DIR}/include ${COM_INC} ${IO_INC})


IF(NOT BUILD_STATIC)
  add_library(SIM SHARED ${LIB_SRCS})
ELSE()
  add_library(SIM STATIC ${LIB_SRCS})
  add_definitions( -DSTATIC_LINK )
ENDIF()
target_link_libraries(SIM SITCOM ${MPI_CXX_LIBRARIES})
SET_TARGET_PROPERTIES(SIM PROPERTIES LINK_FLAGS "${MPI_CXX_LINK_FLAGS}")

#add_executable(testx ${TEST_SRCS})
#target_link_libraries(testx MAN ${APPUTIL_LIB})

#ADD_TEST(RunAllTests ${EXECUTABLE_OUTPUT_PATH}/testx testresults.txt)
#ADD_TEST(TestObject:ValidHandle ${TEST_RESULTS} TestObject:ValidHandle testresults.txt)
#ADD_TEST(TestObject:Exists ${TEST_RESULTS} TestObject:Exists testresults.txt)

INSTALL(TARGETS SIM RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
