cmake_minimum_required(VERSION 3.1)

add_library(SIM
    # Action
    src/Action.C
    src/SchedulerAction.C
    src/Interpolate.C
    # Scheduler
    src/Scheduler.C
    src/DDGScheduler.C
    src/UserScheduler.C
    # Agent
    src/Agent.C
    # Coupling
    src/Coupling.C

    # Helper
    src/RocBlas.C
)

set_target_properties(SIM PROPERTIES VERSION ${IMPACT_VERSION}
        SOVERSION ${IMPACT_MAJOR_VERSION})

target_link_libraries(SIM SITCOM)

target_include_directories(SIM
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/impact>
)

# install the headers and export the targets
install(DIRECTORY include/ 
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/impact)
install(TARGETS SIM
    EXPORT IMPACT
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
