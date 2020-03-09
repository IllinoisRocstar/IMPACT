cmake_minimum_required(VERSION 3.1)

#find_library(DL_LIB dl)

# rpath settings
#set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(SurfUtil
    src/Rocsurf.C
    src/Manifold_2.C
    src/Generic_element_2.C
    src/interpolate_to_centers.C
    src/compute_element_normals.C
    src/compute_element_areas.C
    src/compute_bounded_volumes.C
    src/compute_curvature.C
)

set_target_properties(SurfUtil PROPERTIES VERSION ${IMPACT_VERSION}
        SOVERSION ${IMPACT_MAJOR_VERSION})

target_include_directories(SurfUtil
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/impact>
)
target_link_libraries(SurfUtil SurfMap Simpal)

install(DIRECTORY include/ 
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/impact)
install(TARGETS SurfUtil
    EXPORT IMPACT
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
