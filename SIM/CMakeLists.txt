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

# rpath settings
#set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

target_link_libraries(SIM SITCOM)

#find_path(COM_INC com.h HINTS ../COM/include)
#find_path(IO_INC HDF4.h HINTS ../SimIO/In/include)
#include_directories(include ${CMAKE_BINARY_DIR}/include ${COM_INC} ${IO_INC})
target_include_directories(SIM
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/impact>
)

# install the headers and export the targets
install(DIRECTORY include/ 
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/impact)
install(TARGETS SIM
    EXPORT IMPACT
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
)
