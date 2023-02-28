
cmake_minimum_required(VERSION 3.14...3.23)

set(PROJECT "ThreadPool-C")

project(${PROJECT} VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(THREADPOOL "ThreadPool")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

include(GNUInstallDirs)

if(CMAKE_CXX_COMPILER_LOADED)
  message(STATUS "The C++ compiler ID is: ${CMAKE_CXX_COMPILER_ID}")
  message(STATUS "Is the C++ from GNU? ${CMAKE_COMPILER_IS_GNUCXX}")
  message(STATUS "The C++ compiler version is: ${CMAKE_CXX_COMPILER_VERSION}")
endif()

message(STATUS "Is the C compiler loaded? ${CMAKE_C_COMPILER_LOADED}")

if(CMAKE_C_COMPILER_LOADED)
  message(STATUS "The C compiler ID is: ${CMAKE_C_COMPILER_ID}")
  message(STATUS "Is the C from GNU? ${CMAKE_COMPILER_IS_GNUCC}")
  message(STATUS "The C compiler version is: ${CMAKE_C_COMPILER_VERSION}")
endif()

if(NOT CMAKE_BUILD_TYPE)
  message("Setting CMAKE_BUILD_TYPE to Debug")
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
    "Choose the type of build, options are: Debug Release MinSizeRel RelWithDebInfo" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif(NOT CMAKE_BUILD_TYPE)

MESSAGE(STATUS "CMAKE_BUILD_TYPE is set to ${CMAKE_BUILD_TYPE}")
MESSAGE(STATUS "CMAKE_INSTALL_PREFIX = " ${CMAKE_INSTALL_PREFIX})

# Overwrite install prefix
# SET(CMAKE_INSTALL_PREFIX "C:/msys64/mingw64" CACHE PATH "Install Path Enforced" FORCE)

# Enable Debug Logs in Debug Mode
set(THREADPOOL_DEBUG "ON" CACHE STRING "ENABLE DEBUG PRINT FOR THREADPOOL")

include(CompilerWarnings)

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(compiler_warnings INTERFACE)
set_project_warnings(compiler_warnings)

option(ENABLE_TESTING "Enable the tests" OFF)
option(ENABLE_COVERAGE "Enable a Code Coverage buid." OFF)

if(ENABLE_TESTING OR ENABLE_COVERAGE)
  include(CTest)
endif()

if(ENABLE_COVERAGE)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    include(CodeCoverage)
  else()
    message("Coverage only for GNU|Clang")
    set(ENABLE_COVERAGE OFF)
  endif()
endif()

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)

# Add library
add_subdirectory(${PROJECT_SOURCE_DIR}/src/)

# Add tests
if(ENABLE_TESTING OR ENABLE_COVERAGE)
  enable_testing()
  add_subdirectory(${PROJECT_SOURCE_DIR}/tests)
endif()

# Add Exports and Installs

set(Upstream_VERSION ${CMAKE_PROJECT_VERSION})

set_property(TARGET ${THREADPOOL} PROPERTY VERSION ${Upstream_VERSION})
set_property(TARGET ${THREADPOOL} PROPERTY
  INTERFACE_${THREADPOOL}_MAJOR_VERSION ${CMAKE_PROJECT_VERSION_MAJOR})
set_property(TARGET ${THREADPOOL} PROPERTY
  INTERFACE_${THREADPOOL}_MINOR_VERSION ${CMAKE_PROJECT_VERSION_MINOR})
set_property(TARGET ${THREADPOOL} APPEND PROPERTY
  COMPATIBLE_INTERFACE_STRING ${THREADPOOL}_MAJOR_VERSION
)

install(EXPORT ThreadPoolTargets
  FILE ThreadPoolTargets.cmake
  NAMESPACE ${THREADPOOL}::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${THREADPOOL}
)

include(CMakePackageConfigHelpers)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/${THREADPOOL}/${THREADPOOL}Config.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${THREADPOOL}
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${THREADPOOL}/${THREADPOOL}ConfigVersion.cmake"
  VERSION ${Upstream_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(
  FILES
  ${CMAKE_CURRENT_BINARY_DIR}/${THREADPOOL}/${THREADPOOL}Config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/${THREADPOOL}/${THREADPOOL}ConfigVersion.cmake
  DESTINATION
  ${CMAKE_INSTALL_LIBDIR}/cmake/${THREADPOOL}
)

# If we want our project to also be used from a build directory
# we only have to add the following to the bottom of the top level CMakeLists.txt
export(EXPORT ThreadPoolTargets
  FILE ${CMAKE_CURRENT_BINARY_DIR}/${THREADPOOL}/${THREADPOOL}Targets.cmake
  NAMESPACE ${THREADPOOL}::
)



MESSAGE(STATUS "CMAKE_INSTALL_BINDIR: " ${CMAKE_INSTALL_BINDIR})
MESSAGE(STATUS "CMAKE_INSTALL_PREFIX: " ${CMAKE_INSTALL_PREFIX})
MESSAGE(STATUS "CMAKE_INSTALL_INCLUDEDIR: " ${CMAKE_INSTALL_INCLUDEDIR})

MESSAGE(STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY: " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
MESSAGE(STATUS "CMAKE_LIBRARY_OUTPUT_DIRECTORY: " ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
MESSAGE(STATUS "CMAKE_ARCHIVE_OUTPUT_DIRECTORY: " ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

MESSAGE(STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER})
MESSAGE(STATUS "PROJECT_VERSION_MAJOR: " ${${PROJECT}_VERSION_MAJOR})
MESSAGE(STATUS "PROJECT_VERSION_MINOR: " ${${PROJECT}_VERSION_MINOR})
MESSAGE(STATUS "C++ compiler flags: " ${CMAKE_CXX_FLAGS})
MESSAGE(STATUS "C++ flags, Debug configuration: " ${CMAKE_CXX_FLAGS_DEBUG})
MESSAGE(STATUS "C++ flags, Release configuration: " ${CMAKE_CXX_FLAGS_RELEASE})