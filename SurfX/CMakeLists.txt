cmake_minimum_required (VERSION 2.8)
project(SurfX)

load_cache(../)
IF(NOT ENABLE_MPI)
  add_definitions( -DDUMMY_MPI )
ELSE()
  FIND_PACKAGE(MPI REQUIRED)
  add_definitions( -DMPICH_IGNORE_CXX_SEEK )
  IF(MPI_CXX_COMPILER)
    set (CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})
  ENDIF(MPI_CXX_COMPILER)
ENDIF()

#INCLUDE(CTest)
find_path(COM_INC roccom.h HINTS ../COM/include) 
find_path(SimIN_INC HDF4.h HINTS ../SimIO/In/include) 
find_path(MAP_INC mapbasic.h HINTS ../SurfMap/include)
find_path(SURF_INC Rocsurf.h HINTS ../SurfUtil/include)

set (LIB_SRCS src/Base/RFC_Window_base.C src/Base/RFC_Window_base_IO.C src/Base/RFC_Window_base_IO_tecplot.C 
              src/Base/RFC_Window_base_IO_binary.C src/Overlay/RFC_Window_overlay.C src/Overlay/RFC_Window_overlay_fea.C
              src/Overlay/RFC_Window_overlay_IO.C src/Overlay/Overlay_primitives.C src/Overlay/Overlay_0d.C 
              src/Overlay/Overlay_init.C src/Overlay/Overlay.C src/Overlay/Overlay_IO.C src/Base/Vector_n.C
              src/Transfer/RFC_Window_transfer.C src/Transfer/RFC_Window_transfer_comm.C src/Transfer/Transfer_base.C
              src/Transfer/Transfer_2f.C src/Transfer/Transfer_2n.C src/Rocface.C src/Overlay/Triangulation.C 
              src/Base/writer.C src/Base/rfc_assertions.C)
set (TEST_SRCS test/ex1.C test/ex2.C test/ex3.C test/ex4.C test/ex5.C test/plot3d.C test/reptrans.C test/readsdv.C)
set (UTIL_SRCS util/surfdiver.C util/rfctest.C util/autosurfer.C util/surfjumper.C util/surfextractor.C)
set (ALL_SRCS ${LIB_SRCS} ${TEST_SRCS} ${UTIL_SRCS})

set_source_files_properties(${ALL_SRCS} PROPERTIES COMPILE_FLAGS "-fPIC" )
#set(TEST_SRCS src/MANTest.C)

# rpath settings
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

include_directories(include ${CMAKE_BINARY_DIR}/include ${COM_INC} ${SimIN_INC} ${MAP_INC} ${SURF_INC} include/Base include/Transfer include/Overlay)


IF(NOT BUILD_STATIC)
  add_library(SurfX SHARED ${LIB_SRCS})
ELSE()
  add_library(SurfX STATIC ${LIB_SRCS})
  add_definitions( -DSTATIC_LINK )
ENDIF()
target_link_libraries(SurfX SurfMap SurfUtil SITCOM)

add_executable(ex1 test/ex1.C test/meshio.C)
add_executable(ex2 test/ex2.C test/meshio.C)
add_executable(ex3 test/ex3.C test/meshio.C)
add_executable(ex4 test/ex4.C test/meshio.C)
add_executable(ex5 test/ex5.C test/meshio.C)
add_executable(plot3d test/plot3d.C test/meshio.C)
add_executable(reptrans test/reptrans.C test/meshio.C)
add_executable(readsdv test/readsdv.C test/meshio.C)
target_link_libraries(ex1 SurfX)
target_link_libraries(ex2 SurfX)
target_link_libraries(ex3 SurfX)
target_link_libraries(ex4 SurfX)
target_link_libraries(ex5 SurfX)
target_link_libraries(plot3d SurfX)
target_link_libraries(reptrans SurfX)
target_link_libraries(readsdv SurfX)

add_executable(surfdiver util/surfdiver.C)
target_link_libraries(surfdiver SurfX)
add_executable(rfctest util/rfctest.C)
target_link_libraries(rfctest SurfX)
add_executable(autosurfer util/autosurfer.C)
target_link_libraries(autosurfer SurfX)
add_executable(surfjumper util/surfjumper.C)
target_link_libraries(surfjumper SurfX)
add_executable(surfextractor util/surfextractor.C)
target_link_libraries(surfextractor SurfX)

#ADD_TEST(RunAllTests ${EXECUTABLE_OUTPUT_PATH}/testx testresults.txt)
#ADD_TEST(TestObject:ValidHandle ${TEST_RESULTS} TestObject:ValidHandle testresults.txt)
#ADD_TEST(TestObject:Exists ${TEST_RESULTS} TestObject:Exists testresults.txt)

INSTALL(TARGETS SurfX surfdiver rfctest autosurfer surfjumper surfextractor RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
