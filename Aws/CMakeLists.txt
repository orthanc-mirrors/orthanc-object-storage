cmake_minimum_required(VERSION 2.8)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)

project(OrthancAwsS3Storage)

set(PLUGIN_VERSION "mainline")

include(CheckIncludeFileCXX)

set(ORTHANC_FRAMEWORK_SOURCE "hg" CACHE STRING "orthanc source")
set(ORTHANC_FRAMEWORK_VERSION "1.7.0" CACHE STRING "orthanc framework version")
set(ALLOW_DOWNLOADS ON)

# Download and setup the Orthanc framework
include(${CMAKE_SOURCE_DIR}/../Common/Resources/DownloadOrthancFramework.cmake)

include(${ORTHANC_ROOT}/Resources/CMake/OrthancFrameworkParameters.cmake)

set(ENABLE_GOOGLE_TEST ON)
set(ORTHANC_FRAMEWORK_PLUGIN ON)

include(${ORTHANC_ROOT}/Resources/CMake/OrthancFrameworkConfiguration.cmake)


add_definitions(
    -DHAS_ORTHANC_EXCEPTION=1
    -DORTHANC_ENABLE_LOGGING_PLUGIN=1
    -DAWS_STORAGE_PLUGIN=1
    )
add_definitions(-DPLUGIN_VERSION="${PLUGIN_VERSION}")

include_directories(
  ${ORTHANC_ROOT}
  ${ORTHANC_ROOT}/Core
  ${ORTHANC_ROOT}/Plugins/Include
  ${ORTHANC_ROOT}/Plugins/Samples/Common
  )


find_package(cryptopp CONFIG REQUIRED)
find_package(AWSSDK REQUIRED COMPONENTS s3)

include_directories(${WASTORAGE_INCLUDE_DIR})

set(COMMON_SOURCES
    ${CMAKE_SOURCE_DIR}/../Common/IStoragePlugin.h
    ${CMAKE_SOURCE_DIR}/../Common/EncryptionHelpers.cpp
    ${CMAKE_SOURCE_DIR}/../Common/EncryptionHelpers.h
    ${CMAKE_SOURCE_DIR}/../Common/EncryptionConfigurator.cpp
    ${CMAKE_SOURCE_DIR}/../Common/EncryptionConfigurator.h
    ${ORTHANC_ROOT}/Plugins/Samples/Common/OrthancPluginCppWrapper.cpp

    ${ORTHANC_CORE_SOURCES}
  )

add_library(OrthancAwsS3Storage SHARED
    AwsS3StoragePlugin.cpp
    AwsS3StoragePlugin.h
    ${CMAKE_SOURCE_DIR}/../Common/StoragePlugin.cpp

    ${COMMON_SOURCES}
  )

set_target_properties(OrthancAwsS3Storage PROPERTIES
  VERSION ${PLUGIN_VERSION}
  SOVERSION ${PLUGIN_VERSION}
  )

target_link_libraries(OrthancAwsS3Storage
  PRIVATE
  cryptopp-static
  ${AWSSDK_LINK_LIBRARIES}
  )



add_executable(UnitTests
    ${GOOGLE_TEST_SOURCES}
    ${COMMON_SOURCES}

    ${CMAKE_SOURCE_DIR}/../UnitTestsSources/EncryptionTests.cpp
    ${CMAKE_SOURCE_DIR}/../UnitTestsSources/UnitTestsMain.cpp
    )

target_link_libraries(UnitTests
  PRIVATE
  cryptopp-static
  ${GOOGLE_TEST_LIBRARIES}
  ${AWSSDK_LINK_LIBRARIES}
  )
