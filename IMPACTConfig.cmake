include("${CMAKE_CURRENT_LIST_DIR}/IMPACT.cmake")

# Compute the installation prefix relative to this file.
get_filename_component(_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
if(_IMPORT_PREFIX STREQUAL "/")
  set(_IMPORT_PREFIX "")
endif()

# Set runtime library path
set(IMPACT_RUNTIME_LIBRARY_DIRS "${_IMPORT_PREFIX}")

# Cleanup temporary variables.
set(_IMPORT_PREFIX)
