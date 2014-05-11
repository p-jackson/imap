cmake_minimum_required(VERSION 2.8)
project(imap)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(EXT_PROJECTS_DIR "${CMAKE_SOURCE_DIR}/deps")

enable_testing()

include(cotire)

find_package(Git)

# Find the Boost directory.
# Disable auto link with BOOST_ALL_NO_LIB, we use cmake to do the linking.
# We add Boost to the include path with Boost_INCLUDE_DIR.
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.55 COMPONENTS system REQUIRED)  
add_definitions(-DBOOST_ALL_NO_LIB)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})

include_directories(${imap_SOURCE_DIR})
include_directories(${imap_SOURCE_DIR}/include)
include_directories(${CASABLANCA_INCLUDE_DIR})

# Needed by Boost.ASIO
add_definitions(-D_WIN32_WINNT=0x0501)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
elseif(MINGW)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-field-initializers")
endif()

add_subdirectory(${EXT_PROJECTS_DIR}/casablanca)
add_subdirectory(${EXT_PROJECTS_DIR}/catch)

add_subdirectory(imap)
add_subdirectory(tests)
add_subdirectory(command)
add_subdirectory(include)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(TARGET casablanca PROPERTY FOLDER "deps")
set_property(TARGET catch PROPERTY FOLDER "deps")