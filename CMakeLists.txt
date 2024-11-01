cmake_minimum_required(VERSION 3.19...3.24)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Not ideal to use this global variable, but necessary to make sure that tooling and projects use the same version
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)

# strongly encouraged to enable this globally to avoid conflicts between -Wpedantic being enabled and -std=c++20 and -std=gnu++20 for
# example when compiling with PCH enabled
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_EXTENSIONS ON)

# Add project_options https://github.com/aminya/project_options
include(cmake/CPM.cmake)
cpmaddpackage(
  NAME
  project_options
  GITHUB_REPOSITORY
  aminya/project_options
  VERSION
  0.40.0
  # GIT_TAG main
  DOWNLOAD_ONLY)
if(project_options_ADDED)
  include(${project_options_SOURCE_DIR}/Index.cmake)
endif()

# Set the project name to your project name, my project isn't very descriptive
project(
  raylib_game
  VERSION 0.1.0
  LANGUAGES C CXX)
include(cmake/PreventInSourceBuilds.cmake)

#
# Project Settings
#
get_property(BUILDING_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(BUILDING_MULTI_CONFIG)
  if(NOT CMAKE_BUILD_TYPE)
    # Make sure that all supported configuration types have their associated conan packages available. You can reduce this list to only the
    # configuration types you use, but only if one is not forced-set on the command line for VS
    message(TRACE "Setting up multi-config build types")
    set(CMAKE_CONFIGURATION_TYPES
        Debug Release RelWithDebInfo MinSizeRel
        CACHE STRING "Enabled build types" FORCE)
  else()
    message(TRACE "User chose a specific build type, so we are using that")
    set(CMAKE_CONFIGURATION_TYPES
        ${CMAKE_BUILD_TYPE}
        CACHE STRING "Enabled build types" FORCE)
  endif()
endif()
include(cmake/ProjectOptions.cmake)
include(cmake/MoreProjectOptions.cmake)

#
# Dependencies
#
# 3rd party libraries (raylib)
add_subdirectory(libs)

# ##########################################################################################################################################
# Project
# ##########################################################################################################################################

add_subdirectory(src)

# PackageProject.cmake will be used to make our target installable https://github.com/TheLartians/PackageProject.cmake
cpmaddpackage("gh:TheLartians/PackageProject.cmake@1.11.2")
# https://github.com/TheLartians/Format.cmake
cpmaddpackage(
  NAME
  Format.cmake
  VERSION
  1.8.1
  GITHUB_REPOSITORY
  TheLartians/Format.cmake
  OPTIONS
  # set to yes skip cmake formatting
  "FORMAT_SKIP_CMAKE NO"
  # set to yes skip clang formatting
  "FORMAT_SKIP_CLANG NO"
  # path to exclude (optional, supports regular expressions)
  "CMAKE_FORMAT_EXCLUDE libs|resources|assets|src/resources")

# If MSVC is being used, and ASAN is enabled, we need to set the debugger environment so that it behaves well with MSVC's debugger, and we
# can run the target from visual studio
if(MSVC)
  get_all_installable_targets(all_targets)
  message("all_targets=${all_targets}")
  set_target_properties(${all_targets} PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=$(VC_ExecutablePath_x64);%PATH%")
endif()
