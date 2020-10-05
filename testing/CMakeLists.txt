#Need CMake 3.1 in order to use GTest
cmake_minimum_required(VERSION 3.1)

MESSAGE(STATUS "ACCESSING TEST DIRECTORY: ${CMAKE_CURRENT_SOURCE_DIR}")

set (ENABLE_MPI TRUE CACHE BOOL "Build with MPI Support")
set (TEST_ENV_PATH_OPTIONS "PATH=${CMAKE_BINARY_DIR}/bin:$ENV{PATH}")
set (TEST_ENV_LD_OPTIONS
"LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/lib:\
$ENV{LD_LIBRARY_PATH}")
set (TESTMOD_SRCS COMTest/src/TestModule.C)
set (FTESTMOD_SRCS COMTest/src/ModTestObject.F90 COMTest/src/TestModule.F90)

#Copies data from the source directory to the build directory
file(COPY data
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

#add subdir in for the test results
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/data/testResults)

SET (TEST_DATA ${CMAKE_CURRENT_BINARY_DIR}/data)
SET (TEST_RESULTS ${TEST_DATA}/testResults)
if("${IO_FORMAT}" STREQUAL "CGNS")
  set(format "CGNS")
elseif("${IO_FORMAT}" STREQUAL "HDF4")
  set(format "HDF")
endif()

#adding gtest subdir with its own cmake file
ADD_SUBDIRECTORY(lib/googletest-release-1.8.1)
#include gtest library. gtest_SOURCE_DIR is available from subdir addition
INCLUDE_DIRECTORIES(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR}/include/gtest ${gtest_SOURCE_DIR})

#if(NOT ENABLE_MPI)
#  add_definitions( -DDUMMY_MPI )
#else()
#  find_package(MPI REQUIRED)
#  include_directories(${MPI_INCLUDE_PATH})
#  if(MPI_LIBRARY)
#    set(MPI_LIBRARIES ${MPI_LIBRARY})
#  endif(MPI_LIBRARY)
#  if(MPI_EXTRA_LIBRARY)
#    set(MPI_LIBRARIES ${MPI_LIBRARIES} "${MPI_EXTRA_LIBRARY}")
#  endif(MPI_EXTRA_LIBRARY)
  # Needed for mpich 2
#  add_definitions( -DMPICH_IGNORE_CXX_SEEK )
#endif()

#Libraries for the "dummy" modules
ADD_LIBRARY(COMTESTMOD   ${TESTMOD_SRCS})
ADD_LIBRARY(COMFTESTMOD  ${FTESTMOD_SRCS})

if(ENABLE_MPI)
  foreach(include_dir IN LISTS ${MPI_INCLUDE_PATH})
  target_include_directories(COMTESTMOD 
      PUBLIC
          $<BUILD_INTERFACE:${include_dir}>)
  target_include_directories(COMFTESTMOD 
          PUBLIC
              $<BUILD_INTERFACE:${include_dir}>)
  endforeach()
endif()

TARGET_INCLUDE_DIRECTORIES(COMTESTMOD
  PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/COMTest/include>)
TARGET_INCLUDE_DIRECTORIES(COMFTESTMOD
  PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/COMTest/include>)

TARGET_LINK_LIBRARIES( COMTESTMOD SITCOM SolverUtils ${MPI_CXX_LIBRARIES})
TARGET_LINK_LIBRARIES(COMFTESTMOD SITCOM SolverUtils ${MPI_CXX_LIBRARIES} SITCOMF ${MPI_Fortran_LIBRARIES})

MESSAGE(STATUS "Test Env Options: ${TEST_ENV_PATH_OPTIONS} ${TEST_ENV_LD_OPTIONS}")

#--------------- COM Test Executables ---------------
ADD_EXECUTABLE(runCOMInitTest COMTest/src/COMInitTests.C)
TARGET_LINK_LIBRARIES(runCOMInitTest gtest gtest_main SITCOM)
ADD_EXECUTABLE(runCOMModuleLoadingTest COMTest/src/COMModuleLoadingTests.C)
TARGET_LINK_LIBRARIES(runCOMModuleLoadingTest gtest gtest_main SITCOM SITCOMF COMTESTMOD COMFTESTMOD SolverUtils)
ADD_EXECUTABLE(runCOMFunctionTest COMTest/src/COMModuleFunctionTests.C)
TARGET_LINK_LIBRARIES(runCOMFunctionTest gtest gtest_main SITCOM SITCOMF COMTESTMOD COMFTESTMOD SolverUtils)
ADD_EXECUTABLE(runCOMGetModuleTests COMTest/src/COMGetModuleTests.C)
TARGET_LINK_LIBRARIES(runCOMGetModuleTests gtest gtest_main SITCOM SITCOMF COMTESTMOD COMFTESTMOD SolverUtils)
ADD_EXECUTABLE(runCOMLinearDataTransferTests COMTest/src/COMLinearDataTransferTests.C)
TARGET_LINK_LIBRARIES(runCOMLinearDataTransferTests gtest gtest_main SITCOM SITCOMF SolverUtils)
ADD_EXECUTABLE(runCOMQuadraticDataTransferTests COMTest/src/COMQuadraticDataTransferTests.C)
TARGET_LINK_LIBRARIES(runCOMQuadraticDataTransferTests gtest gtest_main SITCOM SITCOMF SolverUtils)
ADD_EXECUTABLE(runCOMDataItemManagementTests COMTest/src/COMDataItemManagementTests.C)
TARGET_LINK_LIBRARIES(runCOMDataItemManagementTests gtest gtest_main SITCOM COMTESTMOD COMFTESTMOD SITCOMF SolverUtils)

#--------------- SimIO Test Executables ---------------
if("${IO_FORMAT}" STREQUAL "CGNS")
  ADD_EXECUTABLE(runSimInSerialTests SimIOTest/serialReadTests.C)
  TARGET_LINK_LIBRARIES(runSimInSerialTests gtest gtest_main SITCOM SimIN)
  #[[TARGET_INCLUDE_DIRECTORIES(runSimInSerialTests
                            PUBLIC 
                                  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/SimIO/In/include> )]]
  ADD_EXECUTABLE(runSimOutSerialTests SimIOTest/serialWriteTest.C)
  TARGET_LINK_LIBRARIES(runSimOutSerialTests gtest gtest_main SITCOM)
endif()

#--------------- Simpal Test Executables ---------------
#these BLAS tests rely on user input and should only be run in the event
#of a potential computational error. The user must inspect visually that the 
#result is the anticipated answer -MAP 
add_executable(runBlasTest ${CMAKE_CURRENT_SOURCE_DIR}/SimpalTest/blastest.C)
target_link_libraries(runBlasTest Simpal)
ADD_EXECUTABLE(runRepTrans ${CMAKE_CURRENT_SOURCE_DIR}/SurfXTest/reptrans.C)
TARGET_LINK_LIBRARIES(runRepTrans Simpal SurfX SITCOM)

#--------------- Sim Test Executables ---------------
ADD_EXECUTABLE(runSimTest ${CMAKE_CURRENT_SOURCE_DIR}/SIMTest/SchedulerTest.C)
TARGET_LINK_LIBRARIES(runSimTest SIM gtest gtest_main )

#--------------- SurfMap Test Executables ---------------
if("${IO_FORMAT}" STREQUAL "CGNS")
  ADD_EXECUTABLE(runPConnTest ${CMAKE_CURRENT_SOURCE_DIR}/SurfMapTest/pconntest.C)
  TARGET_LINK_LIBRARIES(runPConnTest gtest gtest_main SurfMap SITCOM )
  ADD_EXECUTABLE(runPCommTest ${CMAKE_CURRENT_SOURCE_DIR}/SurfMapTest/pcommtest.C)
  TARGET_LINK_LIBRARIES(runPCommTest gtest gtest_main SurfMap SITCOM )
endif()
ADD_EXECUTABLE(runSurfMapHexBorderTest ${CMAKE_CURRENT_SOURCE_DIR}/SurfMapTest/bordertest_hex.C)
TARGET_LINK_LIBRARIES(runSurfMapHexBorderTest gtest gtest_main SurfMap SITCOM)
ADD_EXECUTABLE(runSurfMapStrcBorderTest ${CMAKE_CURRENT_SOURCE_DIR}/SurfMapTest/bordertest_struc.C)
TARGET_LINK_LIBRARIES(runSurfMapStrcBorderTest gtest gtest_main SurfMap SITCOM)
ADD_EXECUTABLE(runSurfMapGhostHexBorderTest ${CMAKE_CURRENT_SOURCE_DIR}/SurfMapTest/bordertestg_hex.C)
TARGET_LINK_LIBRARIES(runSurfMapGhostHexBorderTest gtest gtest_main SurfMap SITCOM)

#--------------- SurfUtil Test Executables ---------------
if("${IO_FORMAT}" STREQUAL "CGNS")
  ADD_EXECUTABLE(runSurfUtilSerializeTest ${CMAKE_CURRENT_SOURCE_DIR}/SurfUtilTest/serialize.C)
  TARGET_LINK_LIBRARIES(runSurfUtilSerializeTest gtest gtest_main SimOUT Simpal SurfMap SurfUtil SITCOM )
  ADD_EXECUTABLE(runSurfUtilComputeFaceNormals ${CMAKE_CURRENT_SOURCE_DIR}/SurfUtilTest/surfComputeFaceNormalsTest.C)
  TARGET_LINK_LIBRARIES(runSurfUtilComputeFaceNormals gtest gtest_main SurfMap SITCOM SurfUtil SimOUT SimIN)
endif()
ADD_EXECUTABLE(runSurfUtilQuadNormalsTest ${CMAKE_CURRENT_SOURCE_DIR}/SurfUtilTest/surfQuadNormalsTest.C)
TARGET_LINK_LIBRARIES(runSurfUtilQuadNormalsTest gtest gtest_main SITCOM SurfUtil SimOUT)


#--------------- SurfX Test Executables ---------------
ADD_EXECUTABLE(runSurfXOverlayUnstrcStrcTest ${CMAKE_CURRENT_SOURCE_DIR}/SurfXTest/TestUnstrcToStrc.C)
TARGET_LINK_LIBRARIES(runSurfXOverlayUnstrcStrcTest gtest gtest_main SITCOM SurfX SimOUT)
ADD_EXECUTABLE(runSurfXOverlayTriToTriTest ${CMAKE_CURRENT_SOURCE_DIR}/SurfXTest/TestTriToTri.C)
TARGET_LINK_LIBRARIES(runSurfXOverlayTriToTriTest gtest gtest_main SITCOM SurfX SimOUT)
ADD_EXECUTABLE(runSurfXDataTransferTest ${CMAKE_CURRENT_SOURCE_DIR}/SurfXTest/TestDataTransfer.C)
TARGET_LINK_LIBRARIES(runSurfXDataTransferTest gtest gtest_main SITCOM SurfX SimOUT)
ADD_EXECUTABLE(runSurfXCellCenteredTest ${CMAKE_CURRENT_SOURCE_DIR}/SurfXTest/TestCellCentered.C)
TARGET_LINK_LIBRARIES(runSurfXCellCenteredTest gtest gtest_main SITCOM SurfX SimOUT)
if("${IO_FORMAT}" STREQUAL "CGNS")
  ADD_EXECUTABLE(runSurfXReadSdvTest ${CMAKE_CURRENT_SOURCE_DIR}/SurfXTest/readsdv.C)
  TARGET_LINK_LIBRARIES(runSurfXReadSdvTest gtest gtest_main SITCOM SurfX SimOUT)
endif()
#[[ADD_EXECUTABLE(runSurfXRfcTest ${CMAKE_CURRENT_SOURCE_DIR}/SurfXTest/rfctest.C)
TARGET_LINK_LIBRARIES(runSurfXRfcTest gtest gtest_main SITCOM SurfX Simpal SimIN SimOUT SurfMap)]]

#--------------- Parallel Executables ---------------
IF(ENABLE_MPI)
  ADD_DEFINITIONS(-D_IMPACT_PARALLEL_)
  ADD_EXECUTABLE(runMCNTest SurfUtilTest/mcntest.C )
  TARGET_LINK_LIBRARIES(runMCNTest gtest gtest_main SurfUtil)
  ADD_EXECUTABLE(runCOMParallelGetSetTests COMTest/src/COMParallelGetSetCommunicatorTests.C)
  TARGET_LINK_LIBRARIES(runCOMParallelGetSetTests gtest gtest_main SITCOM SITCOMF SolverUtils ${MPI_CXX_LIBRARIES})
  ADD_EXECUTABLE(runCOMParallelModuleLoadingTests COMTest/src/COMParallelModuleLoadingTests.C)
  TARGET_LINK_LIBRARIES(runCOMParallelModuleLoadingTests gtest gtest_main SITCOM SITCOMF COMTESTMOD COMFTESTMOD SolverUtils ${MPI_CXX_LIBRARIES})
  ADD_EXECUTABLE(runSimInParallelTests SimIOTest/parallelReadTests.C)
  TARGET_LINK_LIBRARIES(runSimInParallelTests gtest gtest_main SimIN SimOUT SITCOM SITCOMF SolverUtils ${MPI_CXX_LIBRARIES})
  ADD_EXECUTABLE(runPCommParallelTest SurfMapTest/parallelPCommTest.C)
  TARGET_LINK_LIBRARIES(runPCommParallelTest gtest gtest_main SimIN SimOUT SITCOM SurfMap ${MPI_CXX_LIBRARIES})
  ADD_EXECUTABLE(runSurfParallelTest SurfUtilTest/surfComputeNormalsTest.C)
  TARGET_LINK_LIBRARIES(runSurfParallelTest gtest gtest_main SITCOM SurfUtil ${MPI_CXX_LIBRARIES})
  #[[ADD_EXECUTABLE(SimIOTest SimIOTest/param_outtest.C)
  TARGET_LINK_LIBRARIES(SimIOTest gtest gtest_main SimIO)]]
  foreach(include_dir IN LISTS ${MPI_INCLUDE_PATH})
  target_include_directories(runPCommParallelTest
      PUBLIC
          $<BUILD_INTERFACE:${include_dir}>)
    target_include_directories(runSurfParallelTest
        PUBLIC
            $<BUILD_INTERFACE:${include_dir}>)
    target_include_directories(runSimInParallelTests
        PUBLIC
            $<BUILD_INTERFACE:${include_dir}>)
    target_include_directories(runCOMParallelModuleLoadingTests 
        PUBLIC
            $<BUILD_INTERFACE:${include_dir}>)
    target_include_directories(runCOMParallelGetSetTests 
        PUBLIC
            $<BUILD_INTERFACE:${include_dir}>)
    target_include_directories(runMCNTest 
        PUBLIC
            $<BUILD_INTERFACE:${include_dir}>)
  endforeach()
ENDIF()

#--------------- COM Serial Tests ---------------
ADD_TEST(NAME COM.InitTest 
         COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}" 
         runCOMInitTest "-com-home" ${PROJECT_BINARY_DIR} "-com-v" "9" "-com-h"
         WORKING_DIRECTORY ${TEST_RESULTS})
ADD_TEST(NAME COM.ModuleLoadingTest
         COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
         runCOMModuleLoadingTest "-com-home" ${PROJECT_BINARY_DIR}
         WORKING_DIRECTORY ${TEST_DATA})
ADD_TEST(NAME COM.ModuleFunctionTest
         COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
         runCOMFunctionTest "-com-home" ${PROJECT_BINARY_DIR}
         WORKING_DIRECTORY ${TEST_DATA})
ADD_TEST(NAME COM.GetModuleTests
         COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
         runCOMGetModuleTests "-com-home" ${PROJECT_BINARY_DIR}
         WORKING_DIRECTORY ${TEST_DATA})
ADD_TEST(NAME COM.LinearDataTransferTests
         COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
         runCOMLinearDataTransferTests "-com-home" ${PROJECT_BINARY_DIR}
         WORKING_DIRECTORY ${TEST_DATA})
ADD_TEST(NAME COM.QuadraticDataTransferTests
         COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
         runCOMQuadraticDataTransferTests "-com-home" ${PROJECT_BINARY_DIR}
         WORKING_DIRECTORY ${TEST_DATA})
ADD_TEST(NAME COM.DataItemManagementTests
         COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
         runCOMDataItemManagementTests "-com-home" ${PROJECT_BINARY_DIR}
         WORKING_DIRECTORY ${TEST_DATA})

#--------------- Sim Serial Tests ---------------
ADD_TEST(NAME SIM.Test
         COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
         runSimTest 
         WORKING_DIRECTORY ${TEST_DATA})

#--------------- SimIO Serial Tests ---------------
if("${IO_FORMAT}" STREQUAL "CGNS")
  ADD_TEST(NAME SimIn.SerialTests
           COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
           runSimInSerialTests "-com-home" ${PROJECT_BINARY_DIR}
                               ifluid-grid_00.000000_00001.cgns "WindowTestOneResults"
                               "WindowTestTwoResults" "ControlFileResults" ifluid_in_00.000000.txt
           WORKING_DIRECTORY ${TEST_DATA}/simIO_serial_test_files)
  ADD_TEST(NAME SimOut.SerialTests
           COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
           runSimOutSerialTests "-com-home" ${PROJECT_BINARY_DIR}
                                ifluid_in_00.000000.txt simOutSerialTestResults
           WORKING_DIRECTORY ${TEST_DATA}/simIO_serial_test_files)
  #[[ADD_TEST(NAME simIOParamOutTest
            COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
            SimIOTest ${TEST_DATA}/ACM_Rocflu/ACM_4/Rocflu/Rocin/ifluid_in_00.000000.txt
                      ${TEST_DATA}/ACM_Rocflu/ACM_4/Rocflu/Rocin/SimIOParamOutTestResults)]]
endif()

#--------------- SurfMap Serial Tests ---------------
if("${IO_FORMAT}" STREQUAL "CGNS")
  ADD_TEST(NAME SurfMap.PConnTest
           COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
           runPConnTest "-com-home" ${PROJECT_BINARY_DIR}
                        ifluid-grid_00.000000_00001.cgns PConnTestResults.cgns
           WORKING_DIRECTORY ${TEST_DATA}/simIO_parallel_test_files/cube_4/Rocflu/Rocin)
  ADD_TEST(NAME SurfMap.PCommTest
           COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
           runPCommTest "-com-home" ${PROJECT_BINARY_DIR}
                        ifluid-grid_00.000000_00001.cgns PaneConnWindowTestResults.cgns
           WORKING_DIRECTORY ${TEST_DATA}/simIO_parallel_test_files/cube_4/Rocflu/Rocin)
endif()
ADD_TEST(NAME SurfMap.HexBorderTest
         COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
         runSurfMapHexBorderTest "-com-home" ${PROJECT_BINARY_DIR}
         WORKING_DIRECTORY ${TEST_RESULTS})
ADD_TEST(NAME SurfMap.StrcBorderTest
         COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
         runSurfMapStrcBorderTest "-com-home" ${PROJECT_BINARY_DIR}
         WORKING_DIRECTORY ${TEST_RESULTS})
if("${IO_FORMAT}" STREQUAL "CGNS")
ADD_TEST(NAME SurfMap.GhostHexBorderTest
         COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
         runSurfMapGhostHexBorderTest "-com-home" ${PROJECT_BINARY_DIR} 
         WORKING_DIRECTORY ${TEST_RESULTS})
elseif("${IO_FORMAT}" STREQUAL "HDF4")
ADD_TEST(NAME SurfMap.GhostHexBorderTest
         COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
         runSurfMapGhostHexBorderTest "-com-home" ${PROJECT_BINARY_DIR} "HDF"
         WORKING_DIRECTORY ${TEST_RESULTS})
endif()

#--------------- SurfUtil Serial Tests ---------------
ADD_TEST(NAME SurfUtil.QuadNormalsTest
         COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
         runSurfUtilQuadNormalsTest "-com-home" ${PROJECT_BINARY_DIR}
                                    100 100
         WORKING_DIRECTORY ${TEST_RESULTS})
if("${IO_FORMAT}" STREQUAL "CGNS")
  ADD_TEST(NAME SurfUtil.SerializeTest
           COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
           runSurfUtilSerializeTest "-com-home" ${PROJECT_BINARY_DIR}
                                     ifluid-grid_00.000000_00001.cgns SurfUtilSerializeResults.cgns
           WORKING_DIRECTORY ${TEST_DATA}/simIO_serial_test_files/)
  ADD_TEST(NAME SurfUtil.ComputeFaceNormals
           COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
           runSurfUtilComputeFaceNormals "-com-home" ${PROJECT_BINARY_DIR}
                                         ifluid-grid_00.000000_00001.cgns FaceNormalsOutput.cgns
           WORKING_DIRECTORY ${TEST_DATA}/simIO_serial_test_files/)
  ADD_TEST(NAME SurfUtil.MCN_CGNS_Test
           COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
           runMCNTest "-com-home" ${PROJECT_BINARY_DIR}
                      ifluid-grid_00.000000_00002.cgns
                      ${TEST_RESULTS}/MCN_CGNS_TestResults.cgns  
           WORKING_DIRECTORY ${TEST_DATA}/ACM_Rocflu/ACM_4/Rocflu/Rocin)
elseif("${IO_FORMAT}" STREQUAL "CGNS")
  ADD_TEST(NAME SurfUtil.MCN_HDF_Test
           COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
           runMCNTest "-com-home" ${PROJECT_BINARY_DIR}
                      ifluid_b_03.400000_0001.hdf
                      ${TEST_RESULTS}/MCN_HDF_TestResults.hdf
           WORKING_DIRECTORY ${TEST_DATA}/ACM_Rocflu/ACM_4/Rocflu/Rocout)
endif()

#--------------- SurfX Serial Tests ----------------
if("${IO_FORMAT}" STREQUAL "CGNS")
ADD_TEST(NAME SurfX.OverlayUnstrcStrcTest
         COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
         runSurfXOverlayUnstrcStrcTest "-com-home" ${PROJECT_BINARY_DIR}
         WORKING_DIRECTORY ${TEST_RESULTS})
ADD_TEST(NAME SurfX.TriToTriOverlayTest
         COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
         runSurfXOverlayTriToTriTest "-com-home" ${PROJECT_BINARY_DIR}
                                     squareMeshUnstrcTri50 1 .obj squareMeshUnstrcTri60 1 .obj
         WORKING_DIRECTORY ${TEST_DATA}/TestMeshes)
ADD_TEST(NAME SurfX.DataTransferTest
         COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
         runSurfXDataTransferTest "-com-home" ${PROJECT_BINARY_DIR}
         WORKING_DIRECTORY ${TEST_RESULTS})
ADD_TEST(NAME SurfX.CellCenteredTest
         COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
         runSurfXCellCenteredTest "-com-home" ${PROJECT_BINARY_DIR}
         WORKING_DIRECTORY ${TEST_RESULTS})
ADD_TEST(NAME SurfX.ReadSdvTest
         COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
         runSurfXReadSdvTest "-com-home" ${PROJECT_BINARY_DIR} quad21_4_sdv.cgns quad21 4
         WORKING_DIRECTORY ${TEST_RESULTS})
elseif("${IO_FORMAT}" STREQUAL "HDF4")
ADD_TEST(NAME SurfX.OverlayUnstrcStrcTest
         COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
         runSurfXOverlayUnstrcStrcTest "-com-home" ${PROJECT_BINARY_DIR} "HDF"
         WORKING_DIRECTORY ${TEST_RESULTS})
ADD_TEST(NAME SurfX.TriToTriOverlayTest
         COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
         runSurfXOverlayTriToTriTest "-com-home" ${PROJECT_BINARY_DIR}
                                     squareMeshUnstrcTri50 1 .obj squareMeshUnstrcTri60 1 .obj
                                     "HDF"
         WORKING_DIRECTORY ${TEST_DATA}/TestMeshes)
ADD_TEST(NAME SurfX.DataTransferTest
         COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
         runSurfXDataTransferTest "-com-home" ${PROJECT_BINARY_DIR} "HDF"
         WORKING_DIRECTORY ${TEST_RESULTS})
ADD_TEST(NAME SurfX.CellCenteredTest
         COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
         runSurfXCellCenteredTest "-com-home" ${PROJECT_BINARY_DIR} "HDF"
         WORKING_DIRECTORY ${TEST_RESULTS})
ADD_TEST(NAME SurfX.ReadSdvTest
         COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
         runSurfXReadSdvTest "-com-home" ${PROJECT_BINARY_DIR} quad21_4_sdv.hdf quad21 4
         WORKING_DIRECTORY ${TEST_RESULTS})
endif()

#[[ADD_TEST(NAME SurfX.RfcTest
  COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
    runSurfXRfcTest "-com-home" ${PROJECT_BINARY_DIR} fluid_in_00.000000.txt ifluid_in_00.000000.txt RfcTestOutput
  WORKING_DIRECTORY ${TEST_DATA}/ACM_Rocflu/ACM_4/Rocflu/Rocin)]]

IF(ENABLE_MPI)
  ADD_TEST(NAME COM.ParallelGetSetTests 
           COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}" 
           ${MPIEXEC_EXECUTABLE} -np 4 ${MPIEXEC_PREFLAGS} runCOMParallelGetSetTests ${MPI_EXEC_POSTFLAGS} "-com-home" ${PROJECT_BINARY_DIR}
           WORKING_DIRECTORY ${TEST_DATA})
  ADD_TEST(NAME COM.ParallelModuleLoadingTests 
           COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}" 
           ${MPIEXEC_EXECUTABLE} -np 4 ${MPIEXEC_PREFLAGS} runCOMParallelModuleLoadingTests ${MPI_EXEC_POSTFLAGS} "-com-home" ${PROJECT_BINARY_DIR}
           WORKING_DIRECTORY ${TEST_DATA})
  ADD_TEST(NAME SimIn.ParallelTests
           COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
           ${MPIEXEC_EXECUTABLE} -np 4 ${MPIEXEC_PREFLAGS} runSimInParallelTests ${MPI_EXEC_POSTFLAGS} "-com-home" ${PROJECT_BINARY_DIR}
                                 fluid_in_00.000000.txt "TestTwoResults" TestOneResults.cgns
           WORKING_DIRECTORY ${TEST_DATA}/simIO_parallel_test_files/cube_4/Rocflu/Rocin)
  if("${IO_FORMAT}" STREQUAL "CGNS")
    ADD_TEST(NAME SurfMap.PCommParallelTest
             COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
             ${MPIEXEC_EXECUTABLE} -np 4 ${MPIEXEC_PREFLAGS} runPCommParallelTest ${MPI_EXEC_POSTFLAGS} "-com-home" ${PROJECT_BINARY_DIR}
                                   ifluid-grid_00.000000_0000 PCommParallelTestResults
             WORKING_DIRECTORY ${TEST_DATA}/simIO_parallel_test_files/cube_4/Rocflu/Rocin)
  endif()
  ADD_TEST(NAME SurfUtil.ParallelTest
           COMMAND ${CMAKE_COMMAND} -E env "${TEST_ENV_PATH_OPTIONS}" "${TEST_ENV_LD_OPTIONS}"
           ${MPIEXEC_EXECUTABLE} -np 4 ${MPIEXEC_PREFLAGS} runSurfParallelTest ${MPI_EXEC_POSTFLAGS} "-com-home" ${PROJECT_BINARY_DIR}
           WORKING_DIRECTORY ${TEST_DATA}/simIO_parallel_test_files/cube_4/Rocflu/Rocin)
ENDIF()

# ========= USE IN EXISTING PROJECT ==============
