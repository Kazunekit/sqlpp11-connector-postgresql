# Copyright (c) 2015, Matthijs Möhlmann
# Copyright (c) 2016, Christian David
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#   Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
#   Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 3.4 FATAL_ERROR)
project(sqlpp11-connector-postgresql VERSION 0.1 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
option(ENABLE_TESTS "Build unit tests" ON)
option(CODE_COVERAGE "Enable coverage reporting" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(ConfigPackageLocation lib/cmake/sqlpp-connector-postgresql)

#find_package(Sqlpp11 REQUIRED)
set(sqlpp11_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../sqlpp11/include" CACHE FILEPATH "Path to sqlpp11 includes")

if(NOT EXISTS ${sqlpp11_INCLUDE_DIRS}/sqlpp11/sqlpp11.h)
    message(SEND_ERROR "Can't find file sqlpp11/sqlpp11.h")
    message("Can't find sqlpp11/sqlpp11.h in ${sqlpp11_INCLUDE_DIRS} ")
    message("Please either")
    message("  - git clone https://github.com/rbock/sqlpp11 ${sqlpp11_INCLUDE_DIRS}")
    message("  - download and unzip a current version from https://github.com/rbock/sqlpp11 to ${sqlpp11_INCLUDE_DIRS}")
    message("  - set SQLPP11_INCLUDE_DIR to point to the dir containing sqlpp11/sqlpp11.h")
    message("")
else()
    message("including sqlpp11 from ${sqlpp11_INCLUDE_DIRS}")
endif()

set(include_dir "${PROJECT_SOURCE_DIR}/include")
file(GLOB_RECURSE sqlpp_headers ${include_dir}/*.h ${sqlpp11_INCLUDE_DIRS}/*.h)

find_package(PostgreSQL REQUIRED)

include_directories("${PROJECT_SOURCE_DIR}/include")

add_subdirectory(src)

if (ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/sqlpp11" DESTINATION include COMPONENT Devel)
install(
  FILES
    cmake/Sqlpp-connector-postgresqlConfig.cmake
  DESTINATION
    ${ConfigPackageLocation}
  COMPONENT
    Devel
)
