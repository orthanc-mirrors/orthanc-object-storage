cmake_minimum_required(VERSION 2.8)

project(OrthancAzureBlobStorage)

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
set(ENABLE_MODULE_IMAGES OFF)
set(ENABLE_MODULE_JOBS OFF)
set(ENABLE_MODULE_DICOM OFF)

include(${ORTHANC_FRAMEWORK_ROOT}/../Resources/CMake/OrthancFrameworkConfiguration.cmake)
include(${ORTHANC_FRAMEWORK_ROOT}/../../OrthancServer/Plugins/Samples/Common/OrthancPluginsExports.cmake)

add_definitions(
    -DHAS_ORTHANC_EXCEPTION=1
    -DORTHANC_ENABLE_LOGGING=1
    -DAZURE_STORAGE_PLUGIN=1
    )
add_definitions(-DPLUGIN_VERSION="${PLUGIN_VERSION}")

include_directories(
  ${ORTHANC_FRAMEWORK_ROOT}
  ${ORTHANC_FRAMEWORK_ROOT}/../../OrthancServer/Plugins/Include
  ${ORTHANC_FRAMEWORK_ROOT}/../../OrthancServer/Plugins/Samples/Common
  )


find_package(cryptopp CONFIG REQUIRED)

# Azure stuff (from https://github.com/Microsoft/vcpkg/issues/6277)
if (NOT WIN32)
  find_package(cpprestsdk CONFIG REQUIRED)
  find_path(WASTORAGE_INCLUDE_DIR was/blob.h)
  find_library(WASTORAGE_LIBRARY azurestorage)
  find_package(Boost REQUIRED COMPONENTS log)
  find_library(UUID_LIBRARY uuid)
  find_package(LibXml2 REQUIRED)
else()  # inspired from https://github.com/phongcao/azure-storage-cpp-sas-sample/blob/master/CMakeLists.txt
  find_path(WASTORAGE_INCLUDE_DIR was/blob.h)
  find_library(CPPREST_LIBRARY
    NAMES cpprest cpprest_2_10)
  find_library(WASTORAGE_LIBRARY wastorage)

  set (CMAKE_CXX_STANDARD 11)
  set (CMAKE_CXX_STANDARD_REQUIRED ON)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHa")
  set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
  set (CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")

  add_definitions(-D_NO_WASTORAGE_API=1)   # from https://github.com/Azure/azure-storage-cpp/issues/263
endif()


if (NOT WIN32)
  include_directories(${WASTORAGE_INCLUDE_DIR})
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
  )

add_library(OrthancAzureBlobStorage SHARED
    AzureBlobStoragePlugin.cpp
    AzureBlobStoragePlugin.h
    ${CMAKE_SOURCE_DIR}/../Common/StoragePlugin.cpp

    ${COMMON_SOURCES}
  )

set_target_properties(OrthancAzureBlobStorage PROPERTIES
  VERSION ${PLUGIN_VERSION}
  SOVERSION ${PLUGIN_VERSION}
  )

if (NOT WIN32)

  target_link_libraries(OrthancAzureBlobStorage
    PRIVATE
    cryptopp-static
    ${WASTORAGE_LIBRARY} 
    ${UUID_LIBRARY} 
    ${Boost_LIBRARIES} 
    ${LIBXML2_LIBRARIES} 
    cpprestsdk::cpprest
    )
else()
  target_link_libraries(OrthancAzureBlobStorage
    PRIVATE
    cryptopp-static
    ${OPENSSL_LIBRARY}
    ${WASTORAGE_LIBRARY} 
    ${CPPREST_LIBRARY} 
    Winhttp.lib
    Crypt32.lib
    xmllite.lib
  )

endif()


# add_executable(UnitTests
#     ${GOOGLE_TEST_SOURCES}
#     ${COMMON_SOURCES}

#     ${CMAKE_SOURCE_DIR}/../UnitTestsSources/EncryptionTests.cpp
#     ${CMAKE_SOURCE_DIR}/../UnitTestsSources/UnitTestsMain.cpp
#     )

# target_link_libraries(UnitTests
#   PRIVATE
#   cryptopp-static
#   ${GOOGLE_TEST_LIBRARIES}
#   ${WASTORAGE_LIBRARY} 
#   # ${UUID_LIBRARY} 
#   # ${Boost_LIBRARIES} 
#   # ${LIBXML2_LIBRARIES} 
#   cpprestsdk::cpprest
#   )
