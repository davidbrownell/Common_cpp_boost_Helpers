cmake_minimum_required(VERSION 3.5.0)

project(BoostHelpers_UnitTests LANGUAGES CXX)

set(CMAKE_MODULE_PATH "$ENV{DEVELOPMENT_ENVIRONMENT_CMAKE_MODULE_PATH}")

if(NOT WIN32)
    string(REPLACE ":" ";" CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}")
endif()

set(CppCommon_STATIC_CRT ON CACHE BOOL "" FORCE)
set(CppCommon_NO_ADDRESS_SPACE_LAYOUT_RANDOMIZATION ON CACHE BOOL "" FORCE)

include(BuildHelpers)

enable_testing()

function(Impl)
    get_filename_component(_this_path ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)

    include(BoostCommon)
    include(CommonHelpers)

    include(${_this_path}/../cmake/BoostHelpers.cmake)

    build_tests(
        FILES
            ${_this_path}/Serialization_UnitTest.cpp
            ${_this_path}/SharedObject_UnitTest.cpp
        LINK_LIBRARIES
            BoostHelpers
            CommonHelpers
            ${Boost_LIBRARIES}
    )
endfunction()

Impl()
