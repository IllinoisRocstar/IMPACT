cmake_minimum_required (VERSION 2.8)
project(COM)
ENABLE_LANGUAGE( CXX Fortran )
IF(NOT ENABLE_MPI)
  add_definitions( -DDUMMY_MPI )
ELSE()
  FIND_PACKAGE(MPI REQUIRED)
  add_definitions( -DMPICH_IGNORE_CXX_SEEK )
  IF(MPI_CXX_COMPILER)
    set (CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})
  ENDIF(MPI_CXX_COMPILER)
ENDIF()
#include(FortranCInterface)
#FortranCInterface_HEADER(${PROJECT_BINARY_DIR}/include/FC.h MACRO_NAMESPACE "FC_")
#FortranCInterface_VERIFY(CXX QUIET)
set (BUILD_STATIC FALSE CACHE BOOL "Build static COM library")
set (ENABLE_MPI TRUE CACHE BOOL "Build with MPI Support")
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single directory for all executables.")
set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib CACHE PATH "Single directory for all libraries and archives.")
mark_as_advanced (LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH)

file(GLOB CPP_SRCS "src/*.C")
file(GLOB F90_SRCS "src/*.f90")
set (TESTMOD_SRCS src/TestModule.C)
set (FTESTMOD_SRCS src/ModTestObject.f90 src/TestModule.f90)
set (ALL_SRCS "${F90_SRCS} ${CPP_SRCS}" )
set_source_files_properties(${ALL_SRCS} PROPERTIES COMPILE_FLAGS "-fPIC" )

# rpath settings
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

include_directories( include ${CMAKE_BINARY_DIR}/include ../include ../../SolverUtils/include )


IF(NOT BUILD_STATIC)
  add_library(COMTESTMOD SHARED ${TESTMOD_SRCS})
  add_library(COMFTESTMOD SHARED ${FTESTMOD_SRCS})
ELSE()
  add_library(COMTESTMOD STATIC ${TESTMOD_SRCS})
  add_library(COMFTESTMOD STATIC ${FTESTMOD_SRCS})
  add_definitions( -DSTATIC_LINK )
ENDIF()
ADD_EXECUTABLE(com_test src/TestCOM.C)
TARGET_LINK_LIBRARIES(com_test ${IRAD_LIB} SolverUtils SITCOM)
TARGET_LINK_LIBRARIES(COMTESTMOD ${IRAD_LIB} SolverUtils SITCOM )
TARGET_LINK_LIBRARIES(COMFTESTMOD ${IRAD_LIB} SolverUtils SITCOMF SITCOM)

add_executable(ComTestDriver src/TestModuleDriver.C)
target_link_libraries(ComTestDriver SITCOM)

add_executable(ComTestFDriver src/TestModuleDriver.f90)
set_target_properties(ComTestFDriver PROPERTIES LINKER_LANGUAGE Fortran)
target_link_libraries(ComTestFDriver SITCOMF)



IF(MPI_CXX_COMPILER)
  ADD_DEFINITIONS(-D_IMPACT_PARALLEL)
  ADD_EXECUTABLE(com_parallel_test src/ParallelTestCOM.C)
  TARGET_LINK_LIBRARIES(com_parallel_test ${IRAD_LIB} SolverUtils SITCOM)
ENDIF()


# This line runs the unit tests from the "com_test" executable
ADD_TEST(COM::RunUnitTests ${CMAKE_BINARY_DIR}/bin/com_test -o com_testresults.txt)

# Everything else just checks if the tests from "com_test" succeeded
# 
# COM Runtime tests
#
ADD_TEST(COM::Init:Works ${TESTRESULT} COM::Init:Works com_testresults.txt)
ADD_TEST(COM::Finalize:Works ${TESTRESULT} COM::Finalize:Works com_testresults.txt)
ADD_TEST(COM::Initialized:Works ${TESTRESULT} COM::Initialized:Works com_testresults.txt)

#
# Module management tests
#
ADD_TEST(COM::ModuleLoadingTest:Runs ${TESTRESULT} ModuleLoadingTest:Runs com_testresults.txt)
ADD_TEST(COM::GetModules:Works ${TESTRESULT} COM::GetModules:Works com_testresults.txt)
ADD_TEST(COM::GetWindows:Works ${TESTRESULT} COM::GetWindows:Works com_testresults.txt)
#ADD_TEST(COM::CModuleLoadingMemTest:Runs ${CMAKE_BINARY_DIR}/bin/${TESTRESULT} CModuleLoadingMemTest:Runs com_testresults.txt)
#ADD_TEST(COM::FModuleLoadingMemTest:Runs ${CMAKE_BINARY_DIR}/bin/${TESTRESULT} FModuleLoadingMemTest:Runs com_testresults.txt)
ADD_TEST(COM::LoadModule:WorksForCModule ${TESTRESULT} LoadModule:WorksForCModule com_testresults.txt)
ADD_TEST(COM::LoadModule:WorksForFortranModule ${TESTRESULT} LoadModule:WorksForFortranModule com_testresults.txt)
ADD_TEST(COM::CModuleLoadsCModule ${TESTRESULT} CModuleLoadsCModule com_testresults.txt)
ADD_TEST(COM::CModuleDestroysCModuleOnUnload ${TESTRESULT} CModuleDestroysCModuleOnUnload com_testresults.txt)
ADD_TEST(COM::CModuleLoadsFModule ${TESTRESULT} CModuleLoadsFModule com_testresults.txt)
ADD_TEST(COM::CModuleDestroysFModuleOnUnload ${TESTRESULT} CModuleDestroysFModuleOnUnload com_testresults.txt)
ADD_TEST(COM::FModuleLoadsFModule ${TESTRESULT} FModuleLoadsFModule com_testresults.txt)
ADD_TEST(COM::FModuleDestroysFModuleOnUnload ${TESTRESULT} FModuleDestroysFModuleOnUnload com_testresults.txt)
ADD_TEST(COM::FModuleLoadsCModule ${TESTRESULT} FModuleLoadsCModule com_testresults.txt)
ADD_TEST(COM::FModuleDestroysCModuleOnUnload ${TESTRESULT} FModuleDestroysCModuleOnUnload com_testresults.txt)
ADD_TEST(COM::UnLoadModule:WorksForCModule ${TESTRESULT} UnLoadModule:WorksForCModule com_testresults.txt)
ADD_TEST(COM::UnLoadModule:WorksForFortranModule ${TESTRESULT} UnLoadModule:WorksForFortranModule com_testresults.txt)

#
# Function  Management Tests
#
ADD_TEST(COM::ModuleFunctionTest:Runs ${TESTRESULT} ModuleFunctionTest:Runs com_testresults.txt)
ADD_TEST(COM::CModuleMemberFunction0:Registration ${TESTRESULT} CModuleFunction0:RegistrationSuccessful com_testresults.txt)
ADD_TEST(COM::CModuleMemberFunction1:Registration ${TESTRESULT} CModuleFunction1:RegistrationSuccessful com_testresults.txt)
ADD_TEST(COM::CModuleMemberFunction2:Registration ${TESTRESULT} CModuleFunction2:RegistrationSuccessful com_testresults.txt)
ADD_TEST(COM::CModuleMemberFunction3:Registration ${TESTRESULT} CModuleFunction3:RegistrationSuccessful com_testresults.txt)
ADD_TEST(COM::CModuleMemberFunction4:Registration ${TESTRESULT} CModuleFunction4:RegistrationSuccessful com_testresults.txt)
ADD_TEST(COM::CModuleIntFunction:Registration ${TESTRESULT} CModuleIntFunction:RegistrationSuccessful com_testresults.txt)
ADD_TEST(COM::CModuleConstIntFunction:Registration ${TESTRESULT} CModuleConstIntFunction:RegistrationSuccessful com_testresults.txt)
ADD_TEST(COM::CModuleOptionalArgsFunction:Registration ${TESTRESULT} CModuleOptionalArgsFunction:RegistrationSuccessful com_testresults.txt)
ADD_TEST(COM::CModuleOptionalArgsFunction:Works ${TESTRESULT} CModuleOptionalArgsFunction:Works com_testresults.txt)
ADD_TEST(COM::CModuleIntFunction:CallFromDriver ${TESTRESULT} CModuleIntFunction:Works com_testresults.txt)
ADD_TEST(COM::CModuleConstIntFunction:CallFromDriver  ${TESTRESULT} CModuleConstIntFunction:Works com_testresults.txt)
ADD_TEST(COM::CModuleMemberFunction:CallFromDriver ${TESTRESULT} CModuleFunction0:CallFromDriver com_testresults.txt)
ADD_TEST(COM::FModuleFunction0:Registration ${TESTRESULT} FModuleFunction0:RegistrationSuccessful com_testresults.txt)
ADD_TEST(COM::FModuleFunction1:Registration ${TESTRESULT} FModuleFunction1:RegistrationSuccessful com_testresults.txt)
ADD_TEST(COM::FModuleFunction2:Registration ${TESTRESULT} FModuleFunction2:RegistrationSuccessful com_testresults.txt)
ADD_TEST(COM::FModuleFunction3:Registration ${TESTRESULT} FModuleFunction3:RegistrationSuccessful com_testresults.txt)
ADD_TEST(COM::FModuleFunction4:Registration ${TESTRESULT} FModuleFunction4:RegistrationSuccessful com_testresults.txt)
ADD_TEST(COM::FModuleIntFunction:Registration ${TESTRESULT} FModuleIntFunction:RegistrationSuccessful com_testresults.txt)
ADD_TEST(COM::FModuleConstIntFunction:Registration ${TESTRESULT} FModuleConstIntFunction:RegistrationSuccessful com_testresults.txt)
ADD_TEST(COM::FModuleMemberFunction:CallFromDriver ${TESTRESULT} FModuleMemberFunction:CallFromDriver com_testresults.txt)
ADD_TEST(COM::FModuleIntFunction:CallFromDriver ${TESTRESULT} FModuleIntFunction:Works com_testresults.txt)
ADD_TEST(COM::FModuleConstIntFunction:CallFromDriver  ${TESTRESULT} FModuleConstIntFunction:Works com_testresults.txt)
ADD_TEST(COM::ExternalCCModule:CallFromDriver ${TESTRESULT} ExternalCCModuleFunction:CallFromDriver com_testresults.txt)
ADD_TEST(COM::ExternalCCModule:CallFromCModule ${TESTRESULT} ExternalCCModuleFunction:CallFromCModule com_testresults.txt)
ADD_TEST(COM::ExternalFFModule:CallFromDriver ${TESTRESULT} ExternalFFModuleFunction:CallFromDriver com_testresults.txt)
ADD_TEST(COM::ExternalFFModule:CallFromFModule ${TESTRESULT} ExternalFFModuleFunction:CallFromFModule com_testresults.txt)
ADD_TEST(COM::ExternalCFModule:CallFromDriver ${TESTRESULT} ExternalCFModuleFunction:CallFromDriver com_testresults.txt)
ADD_TEST(COM::ExternalCFModule:CallFromCModule ${TESTRESULT} ExternalCFModuleFunction:CallFromCModule com_testresults.txt)
ADD_TEST(COM::ExternalFCModule:CallFromDriver ${TESTRESULT} ExternalFCModuleFunction:CallFromDriver com_testresults.txt)
ADD_TEST(COM::ExternalFCModule:CallFromFModule ${TESTRESULT} ExternalFCModuleFunction:CallFromFModule com_testresults.txt)


#
# DataItem Management Tests
#
ADD_TEST(COM::DataItemManagementTest:Runs ${TESTRESULT} COM::DataItemManagement:Runs com_testresults.txt)
ADD_TEST(COM::GetDataItems:Works ${TESTRESULT} COM::GetDataItems:Works com_testresults.txt)
ADD_TEST(COM::GetDataItemHandle:Works ${TESTRESULT} COM::GetDataItemHandle:Works com_testresults.txt)
ADD_TEST(COM::GetArray:Works ${TESTRESULT} COM::GetArray:Works com_testresults.txt)
ADD_TEST(COM::GetSize:Works ${TESTRESULT} COM::GetSize:Works com_testresults.txt)
ADD_TEST(COM::GetDataItem:Works ${TESTRESULT} COM::GetDataItem:Works com_testresults.txt)
ADD_TEST(COM::GetNonLocalDataItem:Works ${TESTRESULT} COM::GetNonLocalDataItem:Works com_testresults.txt)


#
# Data Transfer Tests
#
ADD_TEST(COM::DataTransferTest:Runs ${TESTRESULT} COM::DataTransferTest:Runs com_testresults.txt)
ADD_TEST(COM::DataTransferObject:Created ${TESTRESULT} COM::DataTransferObject:Created com_testresults.txt)
ADD_TEST(COM::DataTransferCoordinates:Retrieved ${TESTRESULT} COM::DataTransferCoordinates:Retrieved com_testresults.txt)
ADD_TEST(COM::DataTransferOverlay:Created ${TESTRESULT} COM::DataTransferOverlay:Created com_testresults.txt)
ADD_TEST(COM::DataTransfer::DirectInjection ${TESTRESULT} COM::DataTransfer::DirectInjection com_testresults.txt)

IF(MPI_CXX_COMPILER)
  # The parallel tests are actually run by this command
  ADD_TEST(COM::RunParallelPlatformTests ${RUNTEST} -p ${PROJECT_SOURCE_DIR}/platforms/parallel_platforms -s ${PROJECT_SOURCE_DIR} -b ${CMAKE_BINARY_DIR}/bin -v 3 -o com_testresults.txt)
  # The remainder use "testresults" utility to check the results of the parallel tests run above
  ADD_TEST(COM::ParallelFinalize:Works ${TESTRESULT} COM::ParallelFinalize:Works com_testresults.txt)
  ADD_TEST(COM::ParallelInitialized:Works ${TESTRESULT} COM::ParallelInitialized:Works com_testresults.txt)
  ADD_TEST(COM::GetCommunicator:Runs ${TESTRESULT} COM::GetCommunicator:Runs com_testresults.txt)
  ADD_TEST(COM::GetCommunicator:Works ${TESTRESULT} COM::GetCommunicator:Works com_testresults.txt)
  ADD_TEST(COM::GetSplitCommunicator:Works ${TESTRESULT} COM::GetSplitCommunicator:Works com_testresults.txt)
  ADD_TEST(COM::GetDefaultCommunicator:Runs ${TESTRESULT} COM::GetDefaultCommunicator:Runs com_testresults.txt)
  ADD_TEST(COM::GetDefaultCommunicator:Works ${TESTRESULT} COM::GetDefaultCommunicator:Works com_testresults.txt)
  ADD_TEST(COM::SetDefaultCommunicator:Runs ${TESTRESULT} COM::SetDefaultCommunicator:Runs com_testresults.txt)
  ADD_TEST(COM::SetDefaultCommunicator:Works ${TESTRESULT} COM::SetDefaultCommunicator:Works com_testresults.txt)
  ADD_TEST(COM::WindowGetCommunicator:Runs ${TESTRESULT} COM::WindowGetCommunicator:Runs com_testresults.txt)
  ADD_TEST(COM::WindowGetSplitCommunicator:Works ${TESTRESULT} COM::WindowGetSplitCommunicator:Works com_testresults.txt)
  ADD_TEST(COM::WindowGetCommunicator:Works ${TESTRESULT} COM::WindowGetCommunicator:Works com_testresults.txt)
  ADD_TEST(COM::ParallelLoadModule:Works ${TESTRESULT} COM::ParallelLoadModule:Works com_testresults.txt)
  ADD_TEST(COM::ParallelUnloadModule:Works ${TESTRESULT} COM::ParallelUnloadModule:Works com_testresults.txt)
  ADD_TEST(COM::ParallelFunctionCall:Works ${TESTRESULT} COM::ParallelFunctionCall:Works com_testresults.txt)
  ADD_TEST(COM::FunctionBarrierTest:Runs ${TESTRESULT} COM::FunctionBarrierTest:Runs com_testresults.txt)
  ADD_TEST(COM::FunctionBarrierTest:Works ${TESTRESULT} COM::FunctionBarrierTest:Works com_testresults.txt)
  ADD_TEST(COM::GetCommunicatorModule:Works ${TESTRESULT} COM::GetCommunicatorModule:Work com_testresults.txt)
ENDIF()
