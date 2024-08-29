cmake_minimum_required(VERSION 3.24)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE
      "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING ""
  )
endif()

project(
  yk_util
  VERSION 0.1.0
  LANGUAGES CXX
)

option(YK_YK_UTIL_BUILD_TESTING "build testing" ${PROJECT_IS_TOP_LEVEL})

add_library(yk_util INTERFACE)

add_library(yk::yk_util ALIAS yk_util)

file(GLOB_RECURSE YK_UTIL_HEADERS CONFIGURE_DEPENDS include/*.hpp)
target_sources(yk_util INTERFACE FILE_SET HEADERS BASE_DIRS include FILES ${YK_UTIL_HEADERS})

include(GNUInstallDirs)
target_include_directories(
  yk_util INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(
  yk_util
  PROPERTIES CXX_STANDARD 20
             CXX_STANDARD_REQUIRED ON
             CXX_EXTENSIONS OFF
)

find_package(Boost CONFIG)
if(${Boost_FOUND})
  target_compile_definitions(yk_util INTERFACE YK_UTIL_INCLUDE_BOOST=1)
  target_link_libraries(yk_util INTERFACE Boost::headers)
endif()

install(
  TARGETS yk_util
  EXPORT yk_util-export
  FILE_SET HEADERS
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(
  EXPORT yk_util-export
  FILE yk_utilTargets.cmake
  NAMESPACE yk::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake
)

if(YK_YK_UTIL_BUILD_TESTING)
  if(${Boost_FOUND})
    add_subdirectory(test EXCLUDE_FROM_ALL)
  else()
    message(FATAL_ERROR "Boost.Test is not found, not building tests")
  endif()
endif()
