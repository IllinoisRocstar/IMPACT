cmake_minimum_required (VERSION 3.1)

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

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single directory for all executables.")
set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib CACHE PATH "Single directory for all libraries and archives.")
mark_as_advanced (LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH)

file(GLOB CPP_SRCS "src/*.C")
set (ALL_SRCS "${CPP_SRCS}" )
set_source_files_properties(${ALL_SRCS} PROPERTIES COMPILE_FLAGS "-fPIC ${MPI_CXX_COMPILE_FLAGS}" )
set (LIB_SRCS src/GeoPrimitives.C 
              src/PGeoPrim.C 
              src/Mesh.C 
              src/PMesh.C 
              src/BSMesh.C 
              src/MeshVTK.C 
              src/FEM.C 
              src/MeshUtils.C
              src/ComLine.C 
              src/COMM.C 
              src/Profiler.C 
              src/UnixUtils.C)
# rpath settings
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

#find_path(IMPACT_INC com.h HINTS ../COM/include)
#file(GLOB H_FILES include/*.H)
#file(GLOB h_FILES include/*.h)
#file(GLOB hpp_FILES include/*.hpp)
#set(SolverUtil_INC_FILES ${H_FILES} ${h_FILES} ${hpp_FILES})

#include_directories(include ${IMPACT_INC} ${CMAKE_BINARY_DIR}/include)

add_library(SolverUtils ${LIB_SRCS})
target_include_directories(SolverUtils 
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      #$<INSTALL_INTERFACE:include/impact> 
      )
if(ENABLE_MPI)
  target_link_libraries(SolverUtils ${MPI_CXX_LIBRARIES} SITCOM SITCOMF)
  set_target_properties(SolverUtils PROPERTIES LINK_FLAGS "${MPI_CXX_LINK_FLAGS}")
  foreach(include_dir IN LISTS ${MPI_INCLUDE_PATH})
  target_include_directories(SolverUtils 
      PUBLIC
          $<BUILD_INTERFACE:${include_dir}>)
  endforeach()
else()
  target_link_libraries(SolverUtils SITCOM SITCOMF)
endif()

add_executable(wrl2mesh src/wrl2mesh.C)
target_link_libraries(wrl2mesh SolverUtils ${MPI_CXX_LIBRARIES})
add_executable(gg2mesh src/gg2mesh.C)
target_link_libraries(gg2mesh SolverUtils ${MPI_CXX_LIBRARIES})
add_executable(t3d2mesh src/t3d2mesh.C)
target_link_libraries(t3d2mesh SolverUtils ${MPI_CXX_LIBRARIES})
add_executable(s2ps src/s2ps.C)
add_executable(test_2d src/test_2d.C)
target_link_libraries(test_2d SolverUtils ${MPI_CXX_LIBRARIES})
add_executable(test_mtx src/test_mtx.C)
target_link_libraries(test_mtx SolverUtils ${MPI_CXX_LIBRARIES})
add_executable(meshgen2d src/meshgen2d.C)
target_link_libraries(meshgen2d SolverUtils ${MPI_CXX_LIBRARIES})
add_executable(winmanip utils/winmanip.C)
target_link_libraries(winmanip SolverUtils SITCOM ${MPI_CXX_LIBRARIES})
set_target_properties(wrl2mesh PROPERTIES LINK_FLAGS "${MPI_CXX_LINK_FLAGS}")
set_target_properties(gg2mesh PROPERTIES LINK_FLAGS "${MPI_CXX_LINK_FLAGS}")
set_target_properties(t3d2mesh PROPERTIES LINK_FLAGS "${MPI_CXX_LINK_FLAGS}")
set_target_properties(test_2d PROPERTIES LINK_FLAGS "${MPI_CXX_LINK_FLAGS}")
set_target_properties(test_mtx PROPERTIES LINK_FLAGS "${MPI_CXX_LINK_FLAGS}")
set_target_properties(meshgen2d PROPERTIES LINK_FLAGS "${MPI_CXX_LINK_FLAGS}")
set_target_properties(winmanip PROPERTIES LINK_FLAGS "${MPI_CXX_LINK_FLAGS}")

# Find METIS and PARMETIS
find_library(METIS_LIBRARY metis)
find_file(METIS_HDR metis.h)
get_filename_component(METIS_INCLUDE_PATH ${METIS_HDR} PATH)

find_library(PMETIS_LIBRARY parmetis)
find_file(PMETIS_HDR parmetis.h)
get_filename_component(PMETIS_INCLUDE_PATH ${PMETIS_HDR} PATH)

if(METIS_LIBRARY OR PMETIS_LIBRARY)
  target_include_directories(SolverUtils 
    PRIVATE
      $<BUILD_INTERFACE:${METIS_INCLUDE_PATH}>
      #$<INSTALL_INTERFACE:${PMETIS_INCLUDE_PATH}>
      )
  add_definitions( -DUSE_METIS )
  if(PMETIS_LIBRARY)
    target_include_directories(SolverUtils 
      PRIVATE 
        $<BUILD_INTERFACE:${PMETIS_INCLUDE_PATH}>
        #$<INSTALL_INTERFACE:${PMETIS_INCLUDE_PATH}>
        )
    add_definitions( -DPMETIS )
  endif()
endif()

# install the headers and export the targets
#install(DIRECTORY include/ 
#        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/impact)
#install(TARGETS meshgen2d SolverUtils wrl2mesh gg2mesh t3d2mesh 
#        RUNTIME DESTINATION bin 
#        LIBRARY DESTINATION lib 
#        ARCHIVE DESTINATION lib
#        INCLUDES DESTINATION include)
