cmake_minimum_required(VERSION 3.1)

add_library(SurfMap
    src/Rocmap.C
    src/Pane_boundary.C
    src/Pane_connectivity.C
    src/Pane_communicator.C
    src/Dual_connectivity.C
    src/Simple_manifold_2.C
    src/Pane_ghost_connectivity.C
    src/KD_tree_3.C
)

set_target_properties(SurfMap PROPERTIES VERSION ${IMPACT_VERSION}
        SOVERSION ${IMPACT_MAJOR_VERSION})

target_include_directories(SurfMap
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/impact>
)
target_link_libraries(SurfMap SITCOM)

# install the headers and export the targets
install(DIRECTORY include/ 
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/impact)
install(TARGETS SurfMap
    EXPORT IMPACT
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
