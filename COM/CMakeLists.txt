cmake_minimum_required(VERSION 3.1)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries." ON)
option(ENABLE_MPI "Build with MPI support." ON)
option(ENABLE_EPIC "Using IMPACT with EPIC" OFF)

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

# Add preprocessor definition if building static or on Apple system
if(NOT BUILD_SHARED_LIBS)
  add_definitions(-DSTATIC_LINK)
else()
  if(APPLE)
    add_definitions(-DDARWIN)
  endif()
endif()

# Set up Fortran/C Interface API.
include(FortranCInterface)
if(NOT FortranCInterface_GLOBAL_FOUND OR NOT FortranCInterface_MODULE_FOUND)
  message(FATAL_ERROR "Fortran/C Interface not found.")
else()
  FortranCInterface_HEADER(${CMAKE_CURRENT_SOURCE_DIR}/include/FC.h MACRO_NAMESPACE "FC_")
  FortranCInterface_VERIFY(CXX)
endif()

# Construct project targets
add_library(SITCOM
    src/com_c.C
    src/assertion.C
    src/com_exception.C
    src/commpi.C
    src/COM_base.C
    src/DataItem.C
    src/Connectivity.C
    src/ComponentInterface.C
    src/Pane.C
    src/Element_accessors.C
#    src/COM_substrate.C
#    src/ParallelAdapter.C
)
add_library(SITCOMF
    src/com_f.C
    src/m_pointers.F90
    src/utilities.F90
)

file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/include/comf90.h "")
if(ENABLE_EPIC)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/comf90.h.in1 ${CMAKE_CURRENT_SOURCE_DIR}/include/comf90.h COPYONLY)
else(ENABLE_EPIC)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/comf90.h.in2 ${CMAKE_CURRENT_SOURCE_DIR}/include/comf90.h COPYONLY)
endif(ENABLE_EPIC)

# rpath settings
#set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if(ENABLE_MPI)
  if(NOT BUILD_SHARED_LIB)
    target_link_libraries(SITCOM PUBLIC ${CMAKE_DL_LIBS} ${MPI_LIBRARIES})
  else()
    target_link_libraries(SITCOM PUBLIC ${MPI_LIBRARIES})
  endif()
  
  foreach(include_dir IN LISTS ${MPI_INCLUDE_PATH})
  target_include_directories(SITCOM 
      PUBLIC
          $<BUILD_INTERFACE:${include_dir}>
          $<INSTALL_INTERFACE:${include_dir}>)
  endforeach()
  target_compile_definitions(SITCOM 
      PUBLIC
          -DMPICH_IGNORE_CXX_SEEK)

  target_link_libraries(SITCOMF PUBLIC SITCOM ${MPI_LIBRARIES})
  foreach(include_dir IN LISTS ${MPI_INCLUDE_PATH})
    target_include_directories(SITCOMF
        PUBLIC
            $<BUILD_INTERFACE:${include_dir}>
            $<INSTALL_INTERFACE:${include_dir}>)
  endforeach()
  target_compile_definitions(SITCOMF 
      PUBLIC
          -DMPICH_IGNORE_CXX_SEEK)
else()
  target_link_libraries(SITCOMF PUBLIC SITCOM)
  target_compile_definitions(SITCOM PUBLIC -DDUMMY_MPI)
  target_compile_definitions(SITCOMF PUBLIC -DDUMMY_MPI)
endif()

target_include_directories(SITCOM
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/impact>
)
target_include_directories(SITCOMF
    PUBLIC
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:include/impact>
)

#target_link_libraries(SITCOMF SITCOM)
#[[if(NOT BUILD_SHARED_LIB)
  target_link_libraries(SITCOM ${CMAKE_DL_LIBS})
endif()]]

#install(FILES ${COM_INC_FILES} DESTINATION include)
#install(FILES ${CMAKE_BINARY_DIR}/include/FC.h DESTINATION include)
# install the headers and export the targets
install(DIRECTORY include/ 
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/impact)
install(TARGETS SITCOM SITCOMF
        EXPORT IMPACT
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        INCLUDES DESTINATION include
)
