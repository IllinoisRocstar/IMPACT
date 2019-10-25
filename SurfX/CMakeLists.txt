cmake_minimum_required(VERSION 3.1)

# Options
option(ENABLE_CGNS "Build with CGNS support." OFF)

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

# rpath settings
#set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(SurfX
    src/Rocface.C
    src/Base/rfc_assertions.C
    src/Base/RFC_Window_base.C
    src/Base/RFC_Window_base_IO.C
    src/Base/RFC_Window_base_IO_tecplot.C
    src/Base/RFC_Window_base_IO_binary.C
    src/Base/Vector_n.C
    src/Base/writer.C
    src/Overlay/Overlay.C
    src/Overlay/Overlay_0d.C
#    src/Overlay/Overlay_1d.C
    src/Overlay/Overlay_init.C
    src/Overlay/Overlay_IO.C
    src/Overlay/Overlay_primitives.C
    src/Overlay/RFC_Window_overlay.C
    src/Overlay/RFC_Window_overlay_fea.C
    src/Overlay/RFC_Window_overlay_IO.C
    src/Overlay/Triangulation.C
    src/Transfer/RFC_Window_transfer.C
    src/Transfer/RFC_Window_transfer_comm.C
    src/Transfer/Transfer_2f.C
    src/Transfer/Transfer_2n.C
    src/Transfer/Transfer_base.C
)

target_include_directories(SurfX
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/Base>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/Overlay>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/Transfer>
        $<INSTALL_INTERFACE:include/impact>
)
target_link_libraries(SurfX SurfUtil SimIN SimOUT)

if(ENABLE_CGNS)
  target_link_libraries(SurfX ${CGNS_LIBRARY} ${HDF5_LIBRARIES})
  target_include_directories(SurfX
    PUBLIC
      ${CGNS_INCLUDE_PATH}
      ${HDF5_INCLUDE_DIRS}) 
endif()

add_executable(surfdiver util/surfdiver.C)
target_link_libraries(surfdiver SurfX)
add_executable(autosurfer util/autosurfer.C)
target_link_libraries(autosurfer SurfX)
add_executable(surfjumper util/surfjumper.C)
target_link_libraries(surfjumper SurfX)
add_executable(surfextractor util/surfextractor.C)
target_link_libraries(surfextractor SurfX)

# install the headers and export the targets
install(DIRECTORY include/Base/ include/Overlay/ include/Transfer/
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/impact)
install(FILES include/Rocface.h
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/impact)
install(TARGETS SurfX surfdiver autosurfer surfjumper surfextractor
        EXPORT IMPACT
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        INCLUDES DESTINATION include
)
