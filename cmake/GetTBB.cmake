# Copyright (c) 2023, AgiBot Inc.
# All rights reserved.

include(FetchContent)

message(STATUS "get TBB ...")

# 设置本地源码路径（需要根据实际路径修改）
set(SRC_NAME "tbb")
message(STATUS "get ${SRC_NAME} print ...")
message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
set(tbb_LOCAL_SOURCE "${CMAKE_SOURCE_DIR}/_deps/${SRC_NAME}-src" CACHE PATH "Path to local ${SRC_NAME} source")


set(tbb_DOWNLOAD_URL
    "https://github.com/oneapi-src/oneTBB/archive/v2021.13.0.tar.gz"
    CACHE STRING "")

if(tbb_LOCAL_SOURCE)
  FetchContent_Declare(
    tbb
    SOURCE_DIR ${tbb_LOCAL_SOURCE}
    OVERRIDE_FIND_PACKAGE)
else()
  FetchContent_Declare(
    tbb
    URL ${tbb_DOWNLOAD_URL}
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    OVERRIDE_FIND_PACKAGE)
endif()

FetchContent_GetProperties(tbb)
if(NOT tbb_POPULATED)
  set(TBB_TEST
      OFF
      CACHE BOOL "" FORCE)

  set(TBB_EXAMPLES
      OFF
      CACHE BOOL "" FORCE)

  set(BUILD_SHARED_LIBS
      ON
      CACHE BOOL "" FORCE)

  set(TBB_PREFER_STATIC_LIBS
      OFF
      CACHE BOOL "" FORCE)

  set(TBB_DIR
      ""
      CACHE STRING "" FORCE)

  set(TBB_INSTALL
      ON
      CACHE BOOL "")

  set(TBB_STRICT
      OFF
      CACHE BOOL "")

  FetchContent_MakeAvailable(tbb)
endif()

# import targets:
# TBB::tbb
