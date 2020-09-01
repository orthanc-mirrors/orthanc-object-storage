cmake_minimum_required(VERSION 2.8)

project(OrthancGoogleCloudStorage)

set(PLUGIN_VERSION "mainline")

include(CheckIncludeFileCXX)

set(ORTHANC_FRAMEWORK_SOURCE "hg" CACHE STRING "orthanc source")
set(ORTHANC_FRAMEWORK_VERSION "1.7.3" CACHE STRING "orthanc framework version")
set(ALLOW_DOWNLOADS ON)

# Download and setup the Orthanc framework
include(${CMAKE_SOURCE_DIR}/../Common/Resources/DownloadOrthancFramework.cmake)

include(${ORTHANC_FRAMEWORK_ROOT}/../Resources/CMake/OrthancFrameworkParameters.cmake)

set(ENABLE_GOOGLE_TEST ON)
set(ORTHANC_FRAMEWORK_PLUGIN ON)

include(${ORTHANC_FRAMEWORK_ROOT}/../Resources/CMake/OrthancFrameworkConfiguration.cmake)


add_definitions(
    -DHAS_ORTHANC_EXCEPTION=1
    -DORTHANC_ENABLE_LOGGING=1
    -DGOOGLE_STORAGE_PLUGIN=1
    )
add_definitions(-DPLUGIN_VERSION="${PLUGIN_VERSION}")

include_directories(
  ${ORTHANC_FRAMEWORK_ROOT}
  ${ORTHANC_FRAMEWORK_ROOT}/../../OrthancServer/Plugins/Include
  ${ORTHANC_FRAMEWORK_ROOT}/../../OrthancServer/Plugins/Samples/Common
  )

find_package(CURL REQUIRED)
find_package(storage_client REQUIRED)
find_package(cryptopp CONFIG REQUIRED)

set(COMMON_SOURCES
    ${CMAKE_SOURCE_DIR}/../Common/IStoragePlugin.h
    ${CMAKE_SOURCE_DIR}/../Common/BaseStoragePlugin.h
    ${CMAKE_SOURCE_DIR}/../Common/BaseStoragePlugin.cpp
    ${CMAKE_SOURCE_DIR}/../Common/EncryptionHelpers.cpp
    ${CMAKE_SOURCE_DIR}/../Common/EncryptionHelpers.h
    ${CMAKE_SOURCE_DIR}/../Common/EncryptionConfigurator.cpp
    ${CMAKE_SOURCE_DIR}/../Common/EncryptionConfigurator.h
    ${ORTHANC_FRAMEWORK_ROOT}/../../OrthancServer/Plugins/Samples/Common/OrthancPluginCppWrapper.cpp

    ${ORTHANC_CORE_SOURCES}
  )

add_library(OrthancGoogleCloudStorage SHARED
    GoogleStoragePlugin.cpp
    GoogleStoragePlugin.h
    ${CMAKE_SOURCE_DIR}/../Common/StoragePlugin.cpp

    ${COMMON_SOURCES}
  )

set_target_properties(OrthancGoogleCloudStorage PROPERTIES
  VERSION ${PLUGIN_VERSION}
  SOVERSION ${PLUGIN_VERSION}
  )

target_link_libraries(OrthancGoogleCloudStorage
  PRIVATE
  storage_client
  cryptopp-static
  )

add_executable(UnitTests
    ${GOOGLE_TEST_SOURCES}
    ${COMMON_SOURCES}

    ${CMAKE_SOURCE_DIR}/../UnitTestsSources/EncryptionTests.cpp
    ${CMAKE_SOURCE_DIR}/../UnitTestsSources/UnitTestsMain.cpp
    ${CMAKE_SOURCE_DIR}/../UnitTestsSources/UnitTestsGcsClient.cpp
    )

target_link_libraries(UnitTests
  PRIVATE
  cryptopp-static
  storage_client
  ${GOOGLE_TEST_LIBRARIES}
  )
