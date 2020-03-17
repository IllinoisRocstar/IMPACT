cmake_minimum_required(VERSION 3.1)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries." ON)
option(ENABLE_EPIC "Using IMPACT with EPIC" OFF)

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

set_target_properties(SITCOM PROPERTIES VERSION ${IMPACT_VERSION}
        SOVERSION ${IMPACT_MAJOR_VERSION})

add_library(SITCOMF
    src/com_f.C
    src/m_pointers.F90
    src/utilities.F90
)

set_target_properties(SITCOMF PROPERTIES VERSION ${IMPACT_VERSION}
        SOVERSION ${IMPACT_MAJOR_VERSION})

file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/include/comf90.h "")
if(ENABLE_EPIC)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/comf90.h.in1 ${CMAKE_CURRENT_SOURCE_DIR}/include/comf90.h COPYONLY)
else(ENABLE_EPIC)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/comf90.h.in2 ${CMAKE_CURRENT_SOURCE_DIR}/include/comf90.h COPYONLY)
endif(ENABLE_EPIC)

if(ENABLE_MPI)
  target_link_libraries(SITCOM PUBLIC ${CMAKE_DL_LIBS} ${MPI_LIBRARIES})
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
  target_link_libraries(SITCOM PUBLIC ${CMAKE_DL_LIBS})
  target_link_libraries(SITCOMF PUBLIC SITCOM)
  target_compile_definitions(SITCOM PUBLIC -DDUMMY_MPI)
  target_compile_definitions(SITCOMF PUBLIC -DDUMMY_MPI)
endif()

target_include_directories(SITCOM
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/impact>
)
target_include_directories(SITCOMF
    PUBLIC
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/impact>
)

# install the headers and export the targets
install(DIRECTORY include/ 
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/impact)
install(TARGETS SITCOM SITCOMF
        EXPORT IMPACT
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
