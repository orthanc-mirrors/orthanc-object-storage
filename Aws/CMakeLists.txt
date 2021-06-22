# Cloud storage plugins for Orthanc
# Copyright (C) 2020-2021 Osimis S.A., Belgium
#
# This program is free software: you can redistribute it and/or
# modify it under the terms of the GNU Affero General Public License
# as published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.


cmake_minimum_required(VERSION 2.8)

project(OrthancAwsS3Storage)

set(PLUGIN_VERSION "mainline")

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

set(ENABLE_CRYPTO_OPTIONS ON)
set(ENABLE_GOOGLE_TEST ON)
set(ENABLE_MODULE_DICOM OFF)
set(ENABLE_MODULE_IMAGES OFF)
set(ENABLE_MODULE_JOBS OFF)
set(ENABLE_WEB_CLIENT ON)  # Access options related to curl
set(ORTHANC_FRAMEWORK_PLUGIN ON)

include(${ORTHANC_FRAMEWORK_ROOT}/../Resources/CMake/OrthancFrameworkConfiguration.cmake)
include(${ORTHANC_FRAMEWORK_ROOT}/../../OrthancServer/Plugins/Samples/Common/OrthancPluginsExports.cmake)


add_definitions(
  -DHAS_ORTHANC_EXCEPTION=1
  -DORTHANC_ENABLE_LOGGING=1
  -DAWS_STORAGE_PLUGIN=1
  -DPLUGIN_VERSION="${PLUGIN_VERSION}"
  )

include_directories(
  ${ORTHANC_FRAMEWORK_ROOT}
  ${ORTHANC_FRAMEWORK_ROOT}/../../OrthancServer/Plugins/Include
  ${ORTHANC_FRAMEWORK_ROOT}/../../OrthancServer/Plugins/Samples/Common
  )


if (NOT STATIC_BUILD AND USE_VCPKG_PACKAGES)
  # Use vcpkg by Microsoft
  option(BUILD_SHARED_LIBS "Build shared libraries" ON)
  find_package(cryptopp CONFIG REQUIRED)
  find_package(AWSSDK REQUIRED COMPONENTS s3)
  set(CRYPTOPP_LIBRARIES cryptopp-static)
  
else()
  include(${CMAKE_SOURCE_DIR}/../Common/CryptoPPConfiguration.cmake)
  include(${CMAKE_SOURCE_DIR}/AwsStaticConfiguration.cmake)
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

  ${AWS_SOURCES}
  ${CRYPTOPP_SOURCES}
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
  ${CRYPTOPP_LIBRARIES}
  ${AWSSDK_LINK_LIBRARIES}
  ${ORTHANC_FRAMEWORK_LIBRARIES}
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
  ${ORTHANC_FRAMEWORK_LIBRARIES}
  )

if (STATIC_BUILD OR NOT USE_VCPKG_PACKAGES)
  add_dependencies(OrthancAwsS3Storage AwsSdkCpp)
  add_dependencies(UnitTests AwsSdkCpp)
endif()
