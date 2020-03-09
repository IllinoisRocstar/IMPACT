cmake_minimum_required(VERSION 3.1)

add_library(Simpal
    src/Rocblas.C
    src/axpy.C
    src/dots.C
    src/op2args.C
    src/op3args.C
)

set_target_properties(Simpal PROPERTIES VERSION ${IMPACT_VERSION}
        SOVERSION ${IMPACT_MAJOR_VERSION})

# rpath settings
#set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

target_include_directories(Simpal
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/impact>
)
target_link_libraries(Simpal SITCOM)

# install the headers and export the targets
install(DIRECTORY include/ 
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/impact)
install(TARGETS Simpal
    EXPORT IMPACT
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
