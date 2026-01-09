message(STATUS "FindBCUR: Starting. Initial BCUR_INCLUDE_DIR=${BCUR_INCLUDE_DIR}")
find_path(BCUR_INCLUDE_DIR "bcur/bc-ur.hpp")
find_library(BCUR_LIBRARY bcur)

message(STATUS "FindBCUR: After find. BCUR_INCLUDE_DIR=${BCUR_INCLUDE_DIR}")

if (NOT BCUR_INCLUDE_DIR OR NOT BCUR_LIBRARY)
    MESSAGE(STATUS "Could not find installed BCUR, using vendored library instead")
    set(BCUR_VENDORED "ON")
    set(BCUR_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/src/third-party CACHE PATH "Path to BCUR headers" FORCE)
    set(BCUR_LIBRARY bcur_static CACHE STRING "BCUR library name" FORCE)
    message(STATUS "Using vendored BCUR at ${BCUR_INCLUDE_DIR}")
else()
    message(STATUS "Found installed BCUR at ${BCUR_INCLUDE_DIR}")
endif()

message(STATUS "FindBCUR: Final BCUR PATH ${BCUR_INCLUDE_DIR}")
message(STATUS "FindBCUR: Final BCUR LIBRARY ${BCUR_LIBRARY}")