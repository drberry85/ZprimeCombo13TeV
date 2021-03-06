cmake_minimum_required(VERSION 2.6)
project(theta)

option(coverage "build for coverage tests" OFF)
option(lto "build with link-time optimization (only for gcc and release builds)" OFF)
option(cxx11 "build with C++11 (only for gcc and release builds)" OFF)
option(profile "build for profiling (-O0 -pg)" OFF)
option(amdlibm "use amdlibm instead of libm in some places" OFF)
option(llvm "Build llvm plugins" OFF)
option(tbb "Build tbb plugins" OFF)
option(timer "with boost timer" OFF)
option(insource "Put binaries in bin and lib source directories (otherwise, put them in build-dir/theta/{bin,lib})" ON)
option(generic "Use generic C++ implementation of functions; not optimized, platform-specific assembler ones" OFF)

set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib")

string(REPLACE "-O2" "-O3" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})

if(coverage)
   message(STATUS "building for coverage test")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -O0")
   set(CMAKE_BUILD_TYPE Debug)
elseif(profile)
   message(STATUS "building for profiling")
   set(CMAKE_BUILD_TYPE Release) # to enable optimizations
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
else()
   set(CMAKE_BUILD_TYPE RelWithDebInfo)
   string(FIND "${CMAKE_CXX_COMPILER}" "clang++" cl)
   if(cl GREATER -1)
   else()
       if(lto)
            message(STATUS "enabling link-time optimization")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")
            set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -flto")
       endif()
       if(cxx11)
            message(STATUS "enabling cxx11")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
       endif()
   endif()
   message(STATUS "building release version ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
endif()

if(amdlibm)
   message(STATUS "using amdlibm")
   add_definitions(-DUSE_AMDLIBM)
   find_library(amdlibm_lib NAMES amdlibm HINTS "./lib/")
   message("amdlibm_lib: ${amdlibm_lib}")
endif()


if(NOT generic)
  enable_language(ASM-ATT)
  if(NOT "${CMAKE_SYSTEM_PROCESSOR}" MATCHES "x86_64" OR NOT CMAKE_ASM-ATT_COMPILER_WORKS)
      message(STATUS "non-optimized platform / assembler not working; enabling 'generic' option")
      set(generic ON)
  endif()
endif()

if(generic)
   message(STATUS "using generic functions")
   add_definitions(-DGENERIC_ARCH)
else()
   enable_language(ASM-ATT)
endif()

if(timer)
    add_definitions(-DUSE_TIMER)
endif()


# note: set BOOST_ROOT environment variable before executing cmake to set alternate
# boost prefix; you might have to add your boost version to Boost_ADDITIONAL_VERSIONS below.
if (NOT "$ENV{BOOST_ROOT}" STREQUAL "")
set(Boost_NO_SYSTEM_PATHS TRUE)
endif()

set(Boost_ADDITIONAL_VERSIONS "1.52" "1.52.0")
find_package(Boost REQUIRED COMPONENTS regex program_options filesystem system thread timer iostreams)

if(NOT Boost_FOUND)
   message(SEND_ERROR "boost not found")
else()
   message("boost found at include=${Boost_INCLUDE_Dirs}; lib=${Boost_LIBRARY_DIRS}")
endif()

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR})

if(insource)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
else()
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/theta/lib)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/theta/bin)
endif()

add_subdirectory(src)
add_subdirectory(libconfig)
add_subdirectory(plugins)
add_subdirectory(bin)
add_subdirectory(root)

if(llvm)
   message(STATUS "including llvm")
   add_subdirectory(llvm)
endif()

if(tbb)
   message(STATUS "including tbb")
   add_subdirectory(tbb)
endif()

