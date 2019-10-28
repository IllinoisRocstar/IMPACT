cmake_minimum_required(VERSION 3.1)

# rpath settings
#set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

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

target_include_directories(SurfMap
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/impact>
)
target_link_libraries(SurfMap SITCOM)

# install the headers and export the targets
install(DIRECTORY include/ 
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/impact)
install(TARGETS SurfMap
    EXPORT IMPACT
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
)
