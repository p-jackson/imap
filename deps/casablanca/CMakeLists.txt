cmake_minimum_required(VERSION 2.8.8)
project(casablanca_builder CXX)
include(ExternalProject)

# FindGit makes the GIT_EXECUTABLE and GIT_FOUND variables available

if(NOT GIT_FOUND)
  message(FATAL_ERROR "FindGit must be run before including the casablanca project "
          "e.g. find_package(Git)")
endif()

ExternalProject_Add(
  casablanca
  GIT_REPOSITORY https://git01.codeplex.com/casablanca
  GIT_TAG fa40cc31af29
  TIMEOUT 10
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  LOG_DOWNLOAD ON
)

# Expose required variable (CASABLANCA_INCLUDE_DIR) to parent scope
ExternalProject_Get_Property(casablanca source_dir)
set(
  CASABLANCA_INCLUDE_DIR
  ${source_dir}/Release/include
  CACHE INTERNAL "Path to include folder for Casablanca"
)
