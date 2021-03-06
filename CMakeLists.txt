# run it cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=1
# FOLLOWING STRUCTURE OF CMAKE FROM https://stackoverflow.com/questions/16398937/cmake-and-finding-other-projects-and-their-dependencies/16404000
################################################################################

cmake_minimum_required(VERSION 3.2.2)

# project name
project(alias VERSION 0.5)

# add the version config file, this generates something like
#  // the configured options and settings for alias
#  #define alias_VERSION_MAJOR @alias_VERSION_MAJOR@
#  #define alias_VERSION_MINOR @alias_VERSION_MINOR@
#configure_file(alias_config.h.in alias_config.h)
#target_include_directories(alias PUBLIC "${PROJECT_BINARY_DIR}")

option(FORCE_GCC_8 "forces the version 8 of the compiler" ON)

# enable debug messages for now
if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE "Debug")
endif()

include(general_defs.cmake)
include(revision_creator.cmake)

# include dependencies as root folder
list(APPEND CMAKE_FIND_ROOT_PATH "${ALIAS_DEP_ROOT}")
include_directories(BEFORE SYSTEM "${ALIAS_DEP_ROOT}/include")
include_directories("${ROOT_PROJECT_DIR}/src/global/include")
link_directories("${ALIAS_DEP_ROOT}/lib")


# include the cmake project utils framework
include(${THIRD_PARTY_DIR}/cmake-project-utils/cmake_configs/global_definitions.cmake)
include(${THIRD_PARTY_DIR}/cmake-project-utils/cmake_configs/testing.cmake)

# enable debug messages for now
add_definitions(-DDEBUG_LOG_ENABLE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# read libs from the current folder also
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
SET(CMAKE_INSTALL_RPATH ".")


# add google test third party
# HACK to avoid installing google test in here
add_subdirectory(third_party/googletest EXCLUDE_FROM_ALL)



################################
# Unit Tests
################################
enable_global_testing()

# we want to setup the output of all libs and executables to the same place in windows
# to test it easily
if(WIN32)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif(WIN32)

# When done tweaking common stuff, configure the components (subprojects).
# NOTE: The order matters! The most independent ones should go first.
if (WIN32)
	# we do not need uuid here since we use directly the crossuid
else(WIN32)
	add_subdirectory(src/uuid)
endif(WIN32)
add_subdirectory(src/toolbox)
add_subdirectory(src/encryption)
add_subdirectory(src/data)
add_subdirectory(src/actions)
add_subdirectory(src/protos)
add_subdirectory(src/storage)
add_subdirectory(src/service)
add_subdirectory(src/qt_client)

# configure the install target
include(install_targets.cmake)
