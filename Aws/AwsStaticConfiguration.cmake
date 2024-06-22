# Cloud storage plugins for Orthanc
# Copyright (C) 2020-2023 Osimis S.A., Belgium
# Copyright (C) 2024-2024 Orthanc Team SRL, Belgium
# Copyright (C) 2021-2024 Sebastien Jodogne, ICTEAM UCLouvain, Belgium
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


# The versions below must match those in "./DownloadStaticPackages.sh"
SET(AWS_CHECKSUMS_VERSION      0.1.17)
SET(AWS_CRT_CPP_VERSION        0.24.1)
SET(AWS_C_AUTH_VERSION         0.7.1)
SET(AWS_C_CAL_VERSION          0.6.1)
SET(AWS_C_COMMON_VERSION       0.9.3)
SET(AWS_C_COMPRESSION_VERSION  0.2.17)
SET(AWS_C_EVENT_STREAM_VERSION 0.3.1)
SET(AWS_C_HTTP_VERSION         0.7.11)
SET(AWS_C_IO_VERSION           0.13.31)
SET(AWS_C_MQTT_VERSION         0.9.5)
SET(AWS_C_S3_VERSION           0.3.14)
SET(AWS_C_SDKUTILS_VERSION     0.1.11)
SET(AWS_SDK_CPP_VERSION        1.11.178)

SET(AWS_CHECKSUMS_MD5       "c256144404dc74349a8344662111e353")
SET(AWS_CRT_CPP_MD5         "fa2cda44386bd56f1d4609c6a54a59f9")
SET(AWS_C_AUTH_MD5          "aa690622a5f697d47ca3095ba2ef3cca")
SET(AWS_C_CAL_MD5           "6d7c05e2f1c173b923e77b6021cb660d")
SET(AWS_C_COMMON_MD5        "d872ff682d5beca8ada6b19464235ca0")
SET(AWS_C_COMPRESSION_MD5   "b8bc8bc9cf3749eb2d3f36098c9ead27")
SET(AWS_C_EVENT_STREAM_MD5  "e3261d89598a3331eebcd0476b523c73")
SET(AWS_C_HTTP_MD5          "b8ef3268d4d64b7fabe743b03518d2d7")
SET(AWS_C_IO_MD5            "9ae415cb81404e1b76d8e8267e585e34")
SET(AWS_C_MQTT_MD5          "76d59d9f14f3999b0f9618e386e71db6")
SET(AWS_C_S3_MD5            "564bb0df1184ebe9ce41c23a0c18cf1a")
SET(AWS_C_SDKUTILS_MD5      "713cac3392aa20d5dda852cf21ac1957")
SET(AWS_SDK_CPP_MD5         "a3f45888e939bb71506e0f7eaa630e48")


SET(AWS_CHECKSUMS_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-checksums-${AWS_CHECKSUMS_VERSION})
SET(AWS_CHECKSUMS_URL "https://orthanc.uclouvain.be/downloads/third-party-downloads/aws/aws-checksums-${AWS_CHECKSUMS_VERSION}.tar.gz")
DownloadPackage(${AWS_CHECKSUMS_MD5} ${AWS_CHECKSUMS_URL} "${AWS_CHECKSUMS_SOURCES_DIR}")

SET(AWS_CRT_CPP_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-crt-cpp-${AWS_CRT_CPP_VERSION})
SET(AWS_CRT_CPP_URL "https://orthanc.uclouvain.be/downloads/third-party-downloads/aws/aws-crt-cpp-${AWS_CRT_CPP_VERSION}.tar.gz")
DownloadPackage(${AWS_CRT_CPP_MD5} ${AWS_CRT_CPP_URL} "${AWS_CRT_CPP_SOURCES_DIR}")

SET(AWS_C_AUTH_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-auth-${AWS_C_AUTH_VERSION})
SET(AWS_C_AUTH_URL "https://orthanc.uclouvain.be/downloads/third-party-downloads/aws/aws-c-auth-${AWS_C_AUTH_VERSION}.tar.gz")
DownloadPackage(${AWS_C_AUTH_MD5} ${AWS_C_AUTH_URL} "${AWS_C_AUTH_SOURCES_DIR}")

SET(AWS_C_CAL_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-cal-${AWS_C_CAL_VERSION})
SET(AWS_C_CAL_URL "https://orthanc.uclouvain.be/downloads/third-party-downloads/aws/aws-c-cal-${AWS_C_CAL_VERSION}.tar.gz")
DownloadPackage(${AWS_C_CAL_MD5} ${AWS_C_CAL_URL} "${AWS_C_CAL_SOURCES_DIR}")

SET(AWS_C_COMMON_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-common-${AWS_C_COMMON_VERSION})
SET(AWS_C_COMMON_URL "https://orthanc.uclouvain.be/downloads/third-party-downloads/aws/aws-c-common-${AWS_C_COMMON_VERSION}.tar.gz")
DownloadPackage(${AWS_C_COMMON_MD5} ${AWS_C_COMMON_URL} "${AWS_C_COMMON_SOURCES_DIR}")

SET(AWS_C_COMPRESSION_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-compression-${AWS_C_COMPRESSION_VERSION})
SET(AWS_C_COMPRESSION_URL "https://orthanc.uclouvain.be/downloads/third-party-downloads/aws/aws-c-compression-${AWS_C_COMPRESSION_VERSION}.tar.gz")
DownloadPackage(${AWS_C_COMPRESSION_MD5} ${AWS_C_COMPRESSION_URL} "${AWS_C_COMPRESSION_SOURCES_DIR}")

SET(AWS_C_EVENT_STREAM_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-event-stream-${AWS_C_EVENT_STREAM_VERSION})
SET(AWS_C_EVENT_STREAM_URL "https://orthanc.uclouvain.be/downloads/third-party-downloads/aws/aws-c-event-stream-${AWS_C_EVENT_STREAM_VERSION}.tar.gz")
DownloadPackage(${AWS_C_EVENT_STREAM_MD5} ${AWS_C_EVENT_STREAM_URL} "${AWS_C_EVENT_STREAM_SOURCES_DIR}")

SET(AWS_C_HTTP_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-http-${AWS_C_HTTP_VERSION})
SET(AWS_C_HTTP_URL "https://orthanc.uclouvain.be/downloads/third-party-downloads/aws/aws-c-http-${AWS_C_HTTP_VERSION}.tar.gz")
DownloadPackage(${AWS_C_HTTP_MD5} ${AWS_C_HTTP_URL} "${AWS_C_HTTP_SOURCES_DIR}")

SET(AWS_C_IO_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-io-${AWS_C_IO_VERSION})
SET(AWS_C_IO_URL "https://orthanc.uclouvain.be/downloads/third-party-downloads/aws/aws-c-io-${AWS_C_IO_VERSION}.tar.gz")
DownloadPackage(${AWS_C_IO_MD5} ${AWS_C_IO_URL} "${AWS_C_IO_SOURCES_DIR}")

SET(AWS_C_MQTT_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-mqtt-${AWS_C_MQTT_VERSION})
SET(AWS_C_MQTT_URL "https://orthanc.uclouvain.be/downloads/third-party-downloads/aws/aws-c-mqtt-${AWS_C_MQTT_VERSION}.tar.gz")
DownloadPackage(${AWS_C_MQTT_MD5} ${AWS_C_MQTT_URL} "${AWS_C_MQTT_SOURCES_DIR}")

SET(AWS_C_S3_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-s3-${AWS_C_S3_VERSION})
SET(AWS_C_S3_URL "https://orthanc.uclouvain.be/downloads/third-party-downloads/aws/aws-c-s3-${AWS_C_S3_VERSION}.tar.gz")
DownloadPackage(${AWS_C_S3_MD5} ${AWS_C_S3_URL} "${AWS_C_S3_SOURCES_DIR}")

SET(AWS_C_SDKUTILS_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-c-sdkutils-${AWS_C_SDKUTILS_VERSION})
SET(AWS_C_SDKUTILS_URL "https://orthanc.uclouvain.be/downloads/third-party-downloads/aws/aws-c-sdkutils-${AWS_C_SDKUTILS_VERSION}.tar.gz")
DownloadPackage(${AWS_C_SDKUTILS_MD5} ${AWS_C_SDKUTILS_URL} "${AWS_C_SDKUTILS_SOURCES_DIR}")


SET(AWS_SDK_CPP_SOURCES_DIR ${CMAKE_BINARY_DIR}/aws-sdk-cpp-${AWS_SDK_CPP_VERSION})
SET(AWS_SDK_CPP_URL "https://orthanc.uclouvain.be/downloads/third-party-downloads/aws/aws-sdk-cpp-${AWS_SDK_CPP_VERSION}.tar.gz")

if (IS_DIRECTORY "${AWS_SDK_CPP_SOURCES_DIR}")
  set(FirstRun OFF)
else()
  set(FirstRun ON)
endif()

DownloadPackage(${AWS_SDK_CPP_MD5} ${AWS_SDK_CPP_URL} "${AWS_SDK_CPP_SOURCES_DIR}")

if (FirstRun)
  # This is a patch for Microsoft Visual Studio 2015
  execute_process(
    COMMAND ${PATCH_EXECUTABLE} -p0 -N -i
    ${CMAKE_CURRENT_LIST_DIR}/aws-sdk-cpp-${AWS_SDK_CPP_VERSION}.patch
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    RESULT_VARIABLE Failure
    )
endif()


configure_file(
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/include/aws/core/SDKConfig.h.in
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/include/aws/core/SDKConfig.h
  )

configure_file(
  ${AWS_C_COMMON_SOURCES_DIR}/include/aws/common/config.h.in
  ${AWS_C_COMMON_SOURCES_DIR}/include/aws/common/config.h
  )

configure_file(
  ${AWS_CRT_CPP_SOURCES_DIR}/include/aws/crt/Config.h.in
  ${AWS_CRT_CPP_SOURCES_DIR}/include/aws/crt/Config.h
  )


include_directories(
  ${AWS_C_COMMON_SOURCES_DIR}/include/
  ${AWS_C_AUTH_SOURCES_DIR}/include/
  ${AWS_C_CAL_SOURCES_DIR}/include/
  ${AWS_C_COMPRESSION_SOURCES_DIR}/include/
  ${AWS_C_IO_SOURCES_DIR}/include/
  ${AWS_C_HTTP_SOURCES_DIR}/include/
  ${AWS_C_MQTT_SOURCES_DIR}/include/
  ${AWS_C_S3_SOURCES_DIR}/include/
  ${AWS_C_SDKUTILS_SOURCES_DIR}/include/
  ${AWS_C_EVENT_STREAM_SOURCES_DIR}/include/
  ${AWS_CHECKSUMS_SOURCES_DIR}/include/
  ${AWS_CRT_CPP_SOURCES_DIR}/include/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/include/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-transfer/include/
  ${AWS_SDK_CPP_SOURCES_DIR}/generated/src/aws-cpp-sdk-s3/include/
  # ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-s3/include/
  # ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-connect/include/
  )


list(APPEND AWS_SOURCES_SUBDIRS
  ${AWS_C_COMMON_SOURCES_DIR}/source/
  ${AWS_C_COMMON_SOURCES_DIR}/source/external/

  ## C libraries
  
  ${AWS_CHECKSUMS_SOURCES_DIR}/source/
  ${AWS_C_AUTH_SOURCES_DIR}/source/
  ${AWS_C_CAL_SOURCES_DIR}/source/
  ${AWS_C_COMPRESSION_SOURCES_DIR}/source/
  ${AWS_C_EVENT_STREAM_SOURCES_DIR}/source/
  ${AWS_C_HTTP_SOURCES_DIR}/source/
  ${AWS_C_IO_SOURCES_DIR}/source/
  ${AWS_C_MQTT_SOURCES_DIR}/source/
  ${AWS_C_S3_SOURCES_DIR}/source/
  ${AWS_C_SDKUTILS_SOURCES_DIR}/source/

  ## C++ libraries
  
  ${AWS_CRT_CPP_SOURCES_DIR}/source/
  ${AWS_CRT_CPP_SOURCES_DIR}/source/auth/
  ${AWS_CRT_CPP_SOURCES_DIR}/source/crypto/
  ${AWS_CRT_CPP_SOURCES_DIR}/source/endpoints/
  ${AWS_CRT_CPP_SOURCES_DIR}/source/http/
  ${AWS_CRT_CPP_SOURCES_DIR}/source/io/
  ${AWS_CRT_CPP_SOURCES_DIR}/source/external/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-connect/source/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-connect/source/model/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/auth/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/auth/signer/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/auth/signer-provider/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/auth/bearer-token-provider/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/client/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/config/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/config/defaults/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/endpoint/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/endpoint/internal/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/external/cjson/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/external/tinyxml2/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/http/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/http/curl/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/http/standard/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/internal/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/monitoring/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/net/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/smithy/tracing/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/base64/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/component-registry/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/crypto/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/crypto/factory/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/crypto/openssl/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/event/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/json/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/logging/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/memory/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/memory/stl/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/stream/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/threading/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/utils/xml/
  ${AWS_SDK_CPP_SOURCES_DIR}/generated/src/aws-cpp-sdk-s3/source/
  ${AWS_SDK_CPP_SOURCES_DIR}/generated/src/aws-cpp-sdk-s3/source/model/
  ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-transfer/source/transfer/
  )


add_definitions(
  -DAWS_AFFINITY_METHOD=AWS_AFFINITY_METHOD_PTHREAD
  -DBYO_CRYPTO  # To have "aws_tls_server_ctx_new()" defined
  -DENABLE_OPENSSL_ENCRYPTION=1
  -DENABLE_CURL_CLIENT=1
  )

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  list(APPEND AWS_SOURCES_SUBDIRS
    ${AWS_C_COMMON_SOURCES_DIR}/source/windows/
    ${AWS_C_IO_SOURCES_DIR}/source/windows/
    ${AWS_C_IO_SOURCES_DIR}/source/windows/iocp/
    ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/platform/windows/
    )

  add_definitions(
    -DAWS_USE_IO_COMPLETION_PORTS=1
    -DCJSON_AS4CPP_HIDE_SYMBOLS=1    # Disable "dllexport" in cJSON
    )

  set(AWSSDK_LINK_LIBRARIES
    # secur32
    ncrypt
    shlwapi
    userenv
    version
    )
else()
  list(APPEND AWS_SOURCES_SUBDIRS
    #${AWS_C_CAL_SOURCES_DIR}/source/unix/
    #${AWS_SDK_CPP_SOURCES_DIR}/aws-cpp-sdk-core/source/net/linux-shared/
    ${AWS_C_COMMON_SOURCES_DIR}/source/posix/
    ${AWS_C_IO_SOURCES_DIR}/source/linux/
    ${AWS_C_IO_SOURCES_DIR}/source/posix/
    ${AWS_SDK_CPP_SOURCES_DIR}/src/aws-cpp-sdk-core/source/platform/linux-shared/
    )
endif()


foreach(d ${AWS_SOURCES_SUBDIRS})
  aux_source_directory(${d} AWS_SOURCES)
endforeach()


list(APPEND AWS_SOURCES
  ${AWS_C_COMMON_SOURCES_DIR}/source/arch/generic/cpuid.c
  ${AWS_CHECKSUMS_SOURCES_DIR}/source/generic/crc32c_null.c
  )


if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  list(REMOVE_ITEM AWS_SOURCES
    # WARNING: "//" *is* important (don't replace it with "/")
    ${AWS_C_IO_SOURCES_DIR}/source/windows//secure_channel_tls_handler.c
    )
endif()
