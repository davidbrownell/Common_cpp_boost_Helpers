cmake_minimum_required(VERSION 3.5.0)

project(BoostHelpers LANGUAGES CXX)

set(CMAKE_MODULE_PATH "$ENV{DEVELOPMENT_ENVIRONMENT_CMAKE_MODULE_PATH}")

if(NOT WIN32)
    string(REPLACE ":" ";" CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}")
endif()

include(BuildHelpers)

function(Impl)
    get_filename_component(_this_path ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)

    build_library(
        NAME
            BoostHelpers

        IS_INTERFACE
            ON

        FILES
            ${_this_path}/../Serialization.h
            ${_this_path}/../Serialization.suffix.h
            ${_this_path}/../TestHelpers.h

        PUBLIC_INCLUDE_DIRECTORIES
            ${_this_path}/../..
    )
endfunction()

Impl()
