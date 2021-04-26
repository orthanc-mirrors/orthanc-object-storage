cmake_minimum_required(VERSION 2.8)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)

project(OrthancAwsS3Storage)

set(PLUGIN_VERSION "1.2.0")

include(CheckIncludeFileCXX)

set(ORTHANC_FRAMEWORK_SOURCE "hg" CACHE STRING "orthanc source")
set(ORTHANC_FRAMEWORK_VERSION "1.9.2" CACHE STRING "orthanc framework version")
set(USE_VCPKG_PACKAGES ON CACHE BOOL "Use Microsoft vcpkg to link against crypto++ and AWS SDK")
set(STATIC_AWS_CLIENT ON CACHE BOOL "Statically link against AWS client library (only if USE_VCPKG_PACKAGES=OFF)")
set(USE_SYSTEM_CRYPTOPP ON CACHE BOOL "Use the system version of crypto++")

# Download and setup the Orthanc framework
set(ALLOW_DOWNLOADS ON)
include(${CMAKE_SOURCE_DIR}/../Common/Resources/DownloadOrthancFramework.cmake)

include(${ORTHANC_FRAMEWORK_ROOT}/../Resources/CMake/OrthancFrameworkParameters.cmake)

set(ENABLE_WEB_CLIENT ON)  # Access options related to curl
set(ENABLE_GOOGLE_TEST ON)
set(ORTHANC_FRAMEWORK_PLUGIN ON)
set(ENABLE_MODULE_IMAGES OFF)
set(ENABLE_MODULE_JOBS OFF)
set(ENABLE_MODULE_DICOM OFF)

include(${ORTHANC_FRAMEWORK_ROOT}/../Resources/CMake/OrthancFrameworkConfiguration.cmake)
include(${ORTHANC_FRAMEWORK_ROOT}/../../OrthancServer/Plugins/Samples/Common/OrthancPluginsExports.cmake)


add_definitions(
    -DHAS_ORTHANC_EXCEPTION=1
    -DORTHANC_ENABLE_LOGGING=1
    -DAWS_STORAGE_PLUGIN=1
    )
add_definitions(-DPLUGIN_VERSION="${PLUGIN_VERSION}")

include_directories(
  ${ORTHANC_FRAMEWORK_ROOT}
  ${ORTHANC_FRAMEWORK_ROOT}/../../OrthancServer/Plugins/Include
  ${ORTHANC_FRAMEWORK_ROOT}/../../OrthancServer/Plugins/Samples/Common
  )


if (NOT STATIC_BUILD AND USE_VCPKG_PACKAGES)
  # Use vcpkg by Microsoft
  find_package(cryptopp CONFIG REQUIRED)
  find_package(AWSSDK REQUIRED COMPONENTS s3)
  set(CRYPTOPP_LIBRARIES cryptopp-static)
else()
  include(${CMAKE_SOURCE_DIR}/../Common/CryptoPPConfiguration.cmake)
  
  ##
  ## Building the C++ SDK for Amazon AWS
  ## WARNING: This is *not* compatible with Ninja (yet)
  ##
  if (STATIC_AWS_CLIENT)
    set(Flags -DBUILD_SHARED_LIBS=OFF)  # Create static library
  else()
    set(Flags -DBUILD_SHARED_LIBS=ON)
  endif()

  include(ExternalProject)
  externalproject_add(AwsSdkCpp
    GIT_REPOSITORY https://github.com/aws/aws-sdk-cpp
    GIT_TAG 1.8.127

    CMAKE_ARGS
    -DBUILD_ONLY=s3   #-DBUILD_ONLY=s3;transfer
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DENABLE_TESTING=OFF
    ${Flags}

    UPDATE_COMMAND ""    # Don't run "cmake" on AWS each time "make/ninja" is run
    INSTALL_COMMAND ""   # No install
    )

  ExternalProject_Get_Property(AwsSdkCpp SOURCE_DIR)
  include_directories(
    ${SOURCE_DIR}/aws-cpp-sdk-core/include/
    ${SOURCE_DIR}/aws-cpp-sdk-s3/include/
    )

  ExternalProject_Get_Property(AwsSdkCpp BINARY_DIR)
  if (STATIC_AWS_CLIENT)
    set(AWSSDK_LINK_LIBRARIES
      ${BINARY_DIR}/aws-cpp-sdk-s3/libaws-cpp-sdk-s3.a
      ${BINARY_DIR}/aws-cpp-sdk-core/libaws-cpp-sdk-core.a
      ${BINARY_DIR}/.deps/install/lib/libaws-c-event-stream.a
      ${BINARY_DIR}/.deps/install/lib/libaws-checksums.a
      ${BINARY_DIR}/.deps/install/lib/libaws-c-common.a
      curl
      crypto
      )
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      list(APPEND AWSSDK_LINK_LIBRARIES
        gcc   # for "undefined reference to `__cpu_model'" on Ubuntu 16.04
        )
    endif()
  else()
    set(AWSSDK_LINK_LIBRARIES
      ${BINARY_DIR}/aws-cpp-sdk-core/libaws-cpp-sdk-core.so
      ${BINARY_DIR}/aws-cpp-sdk-s3/libaws-cpp-sdk-s3.so
      )
  endif()
endif()


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
    ${CRYPTOPP_SOURCES}
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
  ${CRYPTOPP_LIBRARIES}
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
  ${GOOGLE_TEST_LIBRARIES}
  ${CRYPTOPP_LIBRARIES}
  ${AWSSDK_LINK_LIBRARIES}
  )


if (STATIC_BUILD OR NOT USE_VCPKG_PACKAGES)
  add_dependencies(OrthancAwsS3Storage AwsSdkCpp)
  add_dependencies(UnitTests AwsSdkCpp)
endif()
