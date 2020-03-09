cmake_minimum_required(VERSION 3.1)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries." ON)
option(NOGLOB "Build without glob file search - turn this on to support massive parallelism." OFF)

# Find MPI package or use a dummy.
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

# Find CGNS package
#if(ENABLE_CGNS)
#  find_library(CGNS_LIB cgns)
#  if(NOT CGNS_LIB)
#    message(FATAL_ERROR "CGNS library not found.")
#  endif()
#  find_path(CGNS_INC cgnslib.h)
#  find_file(CGNS_HDR cgnslib.h)
#  get_filename_component(CGNS_INCPATH ${CGNS_HDR} PATH)
#  get_filename_component(CGNS_LIB_PATH ${CGNS_LIB} PATH)
#  list(APPEND LIBRARY_DIRS ${CGNS_LIB_PATH})
#  message(STATUS "cgns include path ${CGNS_INCPATH}")
#  message(STATUS "cgns libs ${CGNS_LIB}")
#  add_definitions(-DUSE_CGNS)
#endif()

if(NOGLOB)
  add_definitions(-D_NO_GLOB_)
endif()

# rpath settings
#set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(SimIN
    src/Rocin.C
    src/read_parameter_file.C
)

set_target_properties(SimIN PROPERTIES VERSION ${IMPACT_VERSION}
        SOVERSION ${IMPACT_MAJOR_VERSION})

if(NOGLOB)
  target_sources(SimIN PRIVATE
      src/Directory.C)
endif()

target_link_libraries(SimIN SITCOM RHDF4)
if(ENABLE_CGNS)
  target_link_libraries(SimIN ${CGNS_LIBRARY} ${HDF5_LIBRARIES})
  target_include_directories(SimIN 
      PUBLIC 
        ${CGNS_INCLUDE_PATH}
        ${HDF5_INCLUDE_DIRS})
endif()


target_include_directories(SimIN
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/impact>
)

if(NOT BUILD_SHARED_LIBS)
  add_definitions(-DSTATIC_LINK)
endif()

# Utilities
#add_executable(hdf2plt util/hdf2plt.C)
#target_link_libraries(hdf2plt SimIN)
#add_executable(printfeas util/printfeas.C)
#target_link_libraries(printfeas SimIN)
#add_executable(hdf2vtk util/hdf2vtk.C)
#target_link_libraries(hdf2vtk SimIN)
##add_executable(hdf2pltV2 util/hdf2pltV2.C)
##target_link_libraries(hdf2pltV2 Rocin)
#add_executable(sepin util/sepin.C)
#target_link_libraries(sepin SimIN)
#add_executable(plagprep util/plagprep.C)
#target_link_libraries(plagprep SimIN)
#add_executable(winstat util/winstat.C)
#target_link_libraries(winstat SimIN)

# install the headers and export the targets
install(DIRECTORY include/ 
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/impact)
install(TARGETS SimIN # hdf2plt printfeas hdf2vtk sepin plagprep
    EXPORT IMPACT
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
