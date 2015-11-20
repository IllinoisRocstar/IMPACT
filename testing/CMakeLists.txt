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


# Project-specific constructs
include_directories(include ${CMAKE_BINARY_DIR}/include)

file(GLOB CPP_SRCS "src/*.C")
SET_SOURCE_FILES_PROPERTIES(${CPP_SRCS} PROPERTIES COMPILE_FLAGS "${MPI_CXX_COMPILE_FLAGS}")
ADD_LIBRARY(IMPACTTest SHARED src/ExampleSourceFile.C)
TARGET_LINK_LIBRARIES(IMPACTTest ${IRAD_LIB} SITCOM ${MPI_CXX_LIBRARIES})
SET_TARGET_PROPERTIES(IMPACTTest PROPERTIES LINK_FLAGS "${MPI_CXX_LINK_FLAGS}")
ADD_EXECUTABLE(impact_test src/TestIMPACT.C)
TARGET_LINK_LIBRARIES(impact_test IMPACTTest ${MPI_CXX_LIBRARIES})
SET_TARGET_PROPERTIES(impact_test PROPERTIES LINK_FLAGS "${MPI_CXX_LINK_FLAGS}")
ADD_EXECUTABLE(sep src/ExampleSerialProgram.C src/sep.C)
TARGET_LINK_LIBRARIES(sep IMPACTTest ${MPI_CXX_LIBRARIES})
SET_TARGET_PROPERTIES(sep PROPERTIES LINK_FLAGS "${MPI_CXX_LINK_FLAGS}")
ADD_EXECUTABLE(make_project src/MakeProject.C)
TARGET_LINK_LIBRARIES(make_project IMPACTTest ${MPI_CXX_LIBRARIES})
SET_TARGET_PROPERTIES(make_project PROPERTIES LINK_FLAGS "${MPI_CXX_LINK_FLAGS}")

IF(MPI_CXX_COMPILER)
  ADD_DEFINITIONS(-D_IMPACT_PARALLEL_)
  ADD_EXECUTABLE(pepi src/ExampleParallelProgram.C src/pepi.C)
  TARGET_LINK_LIBRARIES(pepi IMPACTTest ${MPI_CXX_LIBRARIES})
  SET_TARGET_PROPERTIES(pepi PROPERTIES LINK_FLAGS "${MPI_CXX_LINK_FLAGS}")

  ADD_EXECUTABLE(impact_parallel_test src/ParallelTestIMPACT.C)
  TARGET_LINK_LIBRARIES(impact_parallel_test IMPACTTest ${MPI_CXX_LIBRARIES})
  SET_TARGET_PROPERTIES(impact_parallel_test PROPERTIES LINK_FLAGS "${MPI_CXX_LINK_FLAGS}")

ENDIF()


# The 3 tests below actually run all the tests
ADD_TEST(IMPACT::RunUnitTests ${CMAKE_BINARY_DIR}/bin/impact_test -o impact_testresults.txt)
ADD_TEST(IMPACT::RunTestScripts ${RUNTEST} -l ${PROJECT_SOURCE_DIR}/scripts/tests.list 
         -s ${PROJECT_SOURCE_DIR} -b ${CMAKE_BINARY_DIR}/bin -v 2 -o impact_testresults.txt)
ADD_TEST(IMPACT::RunPlatformSpecificTests ${RUNTEST} -p ${PROJECT_SOURCE_DIR}/platforms/platforms 
         -s ${PROJECT_SOURCE_DIR} -b ${CMAKE_BINARY_DIR}/bin -v 2 -o impact_testresults.txt)
ADD_TEST(IMPACT::RunSpecificTestScript    ${RUNTEST} -f ${PROJECT_SOURCE_DIR}/scripts/test_stub.csh 
         -s ${PROJECT_SOURCE_DIR} -b ${CMAKE_BINARY_DIR}/bin -v 3 -o impact_testresults.txt)

# Simple test of return code of a program
ADD_TEST(IMPACT::ExampleProgram:Runs ${CMAKE_BINARY_DIR}/bin/sep Makefile)

# The remaining "tests" just use the "testresults" utility to check the results of the tests run above
ADD_TEST(IMPACT::TestStubWorks ${TESTRESULT} TestStubWorks impact_testresults.txt)
ADD_TEST(IMPACT::ExampleProgram:Works ${TESTRESULT} ExampleProgram:Works impact_testresults.txt)
ADD_TEST(IMPACT::ExampleFunction:Works ${TESTRESULT} ExampleFunction:Works impact_testresults.txt)
ADD_TEST(IMPACT::TrapezoidQuadrature:Runs ${TESTRESULT} TrapezoidQuadrature:Runs impact_testresults.txt)
ADD_TEST(IMPACT::TrapezoidQuadrature:Accurate ${TESTRESULT} TrapezoidQuadrature:Accurate impact_testresults.txt)
ADD_TEST(IMPACT::TrapezoidQuadrature:Order ${TESTRESULT} TrapezoidQuadrature:Order2 impact_testresults.txt)
ADD_TEST(IMPACT::MidPointQuadrature:Runs ${TESTRESULT} MidPointQuadrature:Runs impact_testresults.txt)
ADD_TEST(IMPACT::MidPointQuadrature:Accurate ${TESTRESULT} MidPointQuadrature:Accurate impact_testresults.txt)
ADD_TEST(IMPACT::MidPointQuadrature:Order ${TESTRESULT} MidPointQuadrature:Order2 impact_testresults.txt)
ADD_TEST(IMPACT::TestModuleDriver:Runs ${TESTRESULT} TestModuleDriver:Runs impact_testresults.txt)
ADD_TEST(IMPACT::TestModuleDriver:Passes ${TESTRESULT} TestModuleDriver:Passes impact_testresults.txt)
ADD_TEST(IMPACT::TestModuleDriverF:Runs ${TESTRESULT} TestModuleDriverF:Runs impact_testresults.txt)
ADD_TEST(IMPACT::TestModuleDriverF:Passes ${TESTRESULT} TestModuleDriverF:Passes impact_testresults.txt)

IF(MPI_CXX_COMPILER)
  # The parallel tests are actually run by this command
  ADD_TEST(IMPACT::RunParallelPlatformTests ${RUNTEST} -p ${PROJECT_SOURCE_DIR}/platforms/parallel_platforms 
           -s ${PROJECT_SOURCE_DIR} -b ${CMAKE_BINARY_DIR}/bin -v 3 -o impact_testresults.txt)
  # The remainder use "testresults" utility to check the results of the parallel tests run above
  ADD_TEST(IMPACT::ParallelExample:Runs ${TESTRESULT} PEPI:Runs impact_testresults.txt)
  ADD_TEST(IMPACT::ParallelExample:Works ${TESTRESULT} PEPI:Works impact_testresults.txt)
  ADD_TEST(IMPACT::ParallelTrapezoidQuadrature:Runs ${TESTRESULT} ParallelTrapezoidQuadrature:Runs impact_testresults.txt)
  ADD_TEST(IMPACT::ParallelTrapezoidQuadrature:Accurate ${TESTRESULT} ParallelTrapezoidQuadrature:Accurate impact_testresults.txt)
  ADD_TEST(IMPACT::ParallelTrapezoidQuadrature:Order ${TESTRESULT} ParallelTrapezoidQuadrature:Order2 impact_testresults.txt)
  ADD_TEST(IMPACT::ParallelTrapezoidQuadrature:WeakScaling ${TESTRESULT} ParallelTrapezoidQuadrature:WeakScaling impact_testresults.txt)
  ADD_TEST(IMPACT::ParallelTrapezoidQuadrature:StrongScaling ${TESTRESULT} ParallelTrapezoidQuadrature:StrongScaling impact_testresults.txt)
  ADD_TEST(IMPACT::ParallelMidPointQuadrature:Runs ${TESTRESULT} ParallelMidPointQuadrature:Runs impact_testresults.txt)
  ADD_TEST(IMPACT::ParallelMidPointQuadrature:Accurate ${TESTRESULT} ParallelMidPointQuadrature:Accurate impact_testresults.txt)
  ADD_TEST(IMPACT::ParallelMidPointQuadrature:Order ${TESTRESULT} ParallelMidPointQuadrature:Order2 impact_testresults.txt)
  ADD_TEST(IMPACT::ParallelMidPointQuadrature:WeakScaling ${TESTRESULT} ParallelMidPointQuadrature:WeakScaling impact_testresults.txt)
  ADD_TEST(IMPACT::ParallelMidPointQuadrature:StrongScaling ${TESTRESULT} ParallelMidPointQuadrature:StrongScaling impact_testresults.txt)
ENDIF()

# ========= USE IN EXISTING PROJECT ==============
