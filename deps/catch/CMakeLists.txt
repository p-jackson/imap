cmake_minimum_required(VERSION 2.8.8)
project(catch_builder CXX)
include(ExternalProject)

# FindGit makes the GIT_EXECUTABLE and GIT_FOUND variables available

if(NOT GIT_FOUND)
  message(FATAL_ERROR "FindGit must be run before including the catch project "
          "e.g. find_package(Git)")
endif()

ExternalProject_Add(
  catch
  GIT_REPOSITORY https://github.com/philsquared/Catch.git
  GIT_TAG af8cd98f20a05828c0101e0344a28cc2a2bf9dd3
  TIMEOUT 10
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  LOG_DOWNLOAD ON
)

# Expose required variable (CATCH_INCLUDE_DIR) to parent scope
ExternalProject_Get_Property(catch source_dir)
set(
  CATCH_INCLUDE_DIR
  ${source_dir}/include
  CACHE INTERNAL "Path to include folder for Catch"
)
